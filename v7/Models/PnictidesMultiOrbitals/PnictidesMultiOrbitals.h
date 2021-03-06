
/** \ingroup SPF */
/*@{*/

/*! \file PnictidesMultiOrbitals
 *
 *  PnictidesMultiOrbitals model
 *
 */
#ifndef PNICTIDES_MULTI_ORBS_H
#define PNICTIDES_MULTI_ORBS_H
#include "PnictidesMultiOrbitalsFields.h"
#include "Random48.h"
#include "ProgressIndicator.h"
#include "Adjustments.h"
#include "SpinOperations.h"
#include "ModelBase.h"
#include "ThreeOrbitalTerms.h"
#include "ObservablesStored.h"
#include "Conductance.h"

namespace Spf {
	template<typename EngineParamsType,
	           typename ParametersModelType_,
	           typename GeometryType,
	           typename ConcurrencyType>
	class PnictidesMultiOrbitals : public ModelBase<Spin<
	  typename EngineParamsType::FieldType>,
	  EngineParamsType,ParametersModelType_,
	  GeometryType,
	  ConcurrencyType> {
		
		typedef typename EngineParamsType::FieldType FieldType;
		typedef std::complex<FieldType> ComplexType;
		typedef PsimagLite::Matrix<ComplexType> MatrixType;
		//typedef RandomNumberGenerator<FieldType> RandomNumberGeneratorType;
		typedef typename GeometryType::PairType PairType;
		typedef Dmrg::ProgressIndicator ProgressIndicatorType;
		typedef Adjustments<EngineParamsType> AdjustmentsType;

		public:

		typedef ParametersModelType_ ParametersModelType;
		static const size_t norb_ = ParametersModelType::numberOfOrbitals;
		typedef PnictidesTwoOrbitalsFields<FieldType,GeometryType> DynVarsType;
		typedef typename DynVarsType::SpinType SpinType;
		typedef typename DynVarsType::SpinOperationsType SpinOperationsType;
		typedef ThreeOrbitalTerms<norb_,MatrixType,ParametersModelType,
				GeometryType> ThreeOrbitalTermsType;
		typedef ObservablesStored<SpinOperationsType,ComplexType,
				ParametersModelType> ObservablesStoredType;
		
		enum {OLDFIELDS,NEWFIELDS};
		enum {SPIN_UP,SPIN_DOWN};
		
		PnictidesMultiOrbitals(const EngineParamsType& engineParams,
		                       const ParametersModelType& mp,
		                       const GeometryType& geometry,
		                       ConcurrencyType)
		: engineParams_(engineParams),
		  mp_(mp),
		  geometry_(geometry),
		  dynVars_(geometry.volume(),
		  engineParams.dynvarsfile),
		  hilbertSize_(2*mp_.numberOfOrbitals*geometry.volume()),
		  adjustments_(engineParams),progress_("PnictidesTwoOrbitals",0),
		  spinOperations_(geometry,engineParams.mcWindow),
		  threeOrbitalTerms_(mp,geometry),
		  observablesStored_(spinOperations_,geometry,mp_,2*mp_.numberOfOrbitals)
		{
		}
		
		DynVarsType& dynVars() { return dynVars_; }
		
		size_t totalFlips() const { return geometry_.volume(); }
		
		SpinOperationsType& ops(SpinOperationsType*) { return spinOperations_; }
		
		size_t hilbertSize() const { return hilbertSize_; }
		
		FieldType deltaDirect(size_t i) const 
		{
			FieldType x = spinOperations_.deltaDirect(i,mp_.jafNn,mp_.jafNnn);
			x += spinOperations_.deltaMagneticField(i,mp_.magneticField);
			return x;
		}
		
		void set(typename DynVarsType::SpinType& dynVars) { spinOperations_.set(dynVars); }
		
		template<typename GreenFunctionType>
		void doMeasurements(GreenFunctionType& greenFunction,size_t iter,std::ostream& fout)
		{
			const SpinType& dynVars = dynVars_.getField((SpinType*)0);
			
			std::string s = "iter=" + ttos(iter); 
			progress_.printline(s,fout);
				
			FieldType temp=greenFunction.calcNumber();
			s ="Number_Of_Electrons="+ttos(temp);
			progress_.printline(s,fout);
			
			//s = "rankGlobal=";
			
			temp=greenFunction.calcElectronicEnergy();
			s="Electronic Energy="+ttos(temp);
			progress_.printline(s,fout);
			
			FieldType temp2=spinOperations_.calcSuperExchange(dynVars,mp_.jafNn);
			s="Superexchange="+ttos(temp2);
			progress_.printline(s,fout);
			
			temp += temp2;
			if (mp_.jafNnn!=0) {
				temp2=spinOperations_.directExchange2(dynVars,mp_.jafNnn);
				s="Superexchange2="+ttos(temp2);
				progress_.printline(s,fout);
				temp += temp2;
			}

			//! total energy = electronic energy + superexchange + phonon energy
			s="TotalEnergy="+ttos(temp);
			progress_.printline(s,fout);
			
			adjustments_.print(fout);
			
			std::vector<FieldType> magVector(3,0);
			spinOperations_.calcMagVector(magVector,dynVars);
			s="ClassicalMagnetizationSquared="+ttos(magVector*magVector);
			progress_.printline(s,fout);

			std::vector<ComplexType> electronSpinVector(3,0);
			greenFunction.electronSpin(electronSpinVector,mp_.numberOfOrbitals,dynVars.size);
			std::vector<ComplexType> combinedVector(3,0);
			combinedVector =  electronSpinVector + magVector;
			s="CombinedMagnetizationSquared="+ttos(std::real(combinedVector*combinedVector));
			progress_.printline(s,fout);
			
			for (size_t i = 0;i<combinedVector.size();i++) {
				s="CombinedMagnetization"+ttos(i)+"="+ttos(combinedVector[i]);
				progress_.printline(s,fout);
			}
			
			if (engineParams_.options.find("conductance")!=std::string::npos) {
				//greenFunction.printMatrix(OLDFIELDS);
				PsimagLite::Matrix<FieldType> v
					(greenFunction.hilbertSize(),greenFunction.hilbertSize());
				calcVelocitySquared(greenFunction,v,GeometryType::DIRX);
				typedef Conductance<EngineParamsType,GreenFunctionType> ConductanceType;
				ConductanceType conductance(engineParams_,greenFunction);
				s = "ConductanceX=" + ttos(conductance(v));
				progress_.printline(s,fout);
				//PsimagLite::Matrix<FieldType> vv = v;
				calcVelocitySquared(greenFunction,v,GeometryType::DIRY);
				s = "ConductanceY=" + ttos(conductance(v));
				//vv -= v;
				//checkMatrix(vv,greenFunction);
				progress_.printline(s,fout);
			}
			
			observablesStored_(dynVars,greenFunction);
		} // doMeasurements
		
// 		template<typename GreenFunctionType>
// 		void checkMatrix(const MatrixType& matrix,GreenFunctionType& gf)
// 		{
// 			size_t volume = geometry_.volume();
// 			size_t norb = mp_.numberOfOrbitals;
// 			double eps = 1e-16;
// 			for (size_t i=0;i<matrix.n_row();i++) {
// 				for (size_t j=0;j<matrix.n_col();j++) {
// 					if (fabs(gf.e(i) -gf.e(j))<eps && norm(matrix(i,j))>eps) {
// 						size_t isite  = i % volume;
// 						size_t tmp = i / volume;
// 						size_t ispin = tmp/norb;
// 						size_t iorb = tmp % norb;
// 						size_t jsite = j % volume;
// 						tmp = j / volume;
// 						size_t jspin = tmp /norb;
// 						size_t jorb = tmp  % norb;
// 						std::cerr<<isite<<" "<<ispin<<" "<<iorb<<" *** "<<jsite<<" "<<jspin<<" "<<jorb<<" m="<<matrix(i,j)<<"\n";
// 					}
// 				}
// 			}
// 		}
		
		void createHamiltonian(MatrixType& matrix,size_t oldOrNewDynVars)
		{
			const SpinType& dynVars = dynVars_.getField((SpinType*)0);
			if (oldOrNewDynVars==NEWFIELDS) createHamiltonian(spinOperations_.dynVars2(),matrix);
			else createHamiltonian(dynVars,matrix);
		}
		
		void adjustChemPot(const std::vector<FieldType>& eigs)
		{
			if (engineParams_.carriers==0) return;
			try {
				engineParams_.mu = adjustments_.adjChemPot(eigs);
			} catch (std::exception& e) {
				std::cerr<<e.what()<<"\n";
			}
				
		}
		
		void accept(size_t i) 
		{
			spinOperations_.accept(i);
		}
		
		FieldType integrationMeasure(size_t i)
		{
			return spinOperations_.sineUpdate(i);
		}
		
		void finalize(std::ostream& fout)
		{
			observablesStored_.finalize(fout);	
		}
		
		template<typename EngineParamsType2,
		         typename ParametersModelType2,
		         typename GeometryType2,
		         typename ConcurrencyType2>
		friend std::ostream& operator<<(std::ostream& os,
		  const PnictidesMultiOrbitals<EngineParamsType2,
		                               ParametersModelType2,
		                               GeometryType2,
		                               ConcurrencyType2>& model);
		
		private:
		
		void createHamiltonian(const typename DynVarsType::SpinType& dynVars,MatrixType& matrix) const
		{
			size_t volume = geometry_.volume();
			size_t norb = mp_.numberOfOrbitals;
			size_t dof = norb * 2; // the 2 comes because of the spin
			std::vector<ComplexType> jmatrix(2*2);
			
			for (size_t gamma1=0;gamma1<matrix.n_row();gamma1++) 
				for (size_t p = 0; p < matrix.n_col(); p++) 
					matrix(gamma1,p)=0;
			
			for (size_t p = 0; p < volume; p++) {
				auxCreateJmatrix(jmatrix,dynVars,p);
				for (size_t gamma1=0;gamma1<dof;gamma1++) {		
					
		
					size_t spin1 = size_t(gamma1/norb);
					size_t orb1 = gamma1 % norb;
					//! Term B (n_iup - n_idown)
					FieldType magField = (spin1==SPIN_UP) ? mp_.magneticField : -mp_.magneticField;
					matrix(p+gamma1*volume,p+gamma1*volume) =
						real(jmatrix[spin1+2*spin1]) + mp_.potentialV[p] + magField;
					for (size_t j = 0; j <  geometry_.z(1); j++) {	
						if (j%2!=0) continue;	
						PairType tmpPair = geometry_.neighbor(p,j);
						size_t k = tmpPair.first;
						size_t dir = tmpPair.second; // int(j/2);
						for (size_t orb2=0;orb2<norb;orb2++) {
							size_t gamma2 = orb2+spin1*norb; // spin2 == spin1 here
							matrix(p+gamma1*volume,k+gamma2*volume)=
								mp_.hoppings[orb1+orb2*norb+norb*norb*dir];
							matrix(k+gamma2*volume,p+gamma1*volume) = conj(matrix(p+gamma1*volume,k+gamma2*volume));
						}
					}
					//if (geometry.z(p,2)!=4 || geometry.z(p)!=4) throw std::runtime_error("neighbours wrong\n");
					for (size_t j = 0; j <  geometry_.z(2); j++) {
						if (j%2!=0) continue;	
						PairType tmpPair = geometry_.neighbor(p,j,2);
						size_t k = tmpPair.first;
						size_t dir = tmpPair.second;
						//std::cerr<<"Neigbors "<<p<<" "<<k<<"\n";
						for (size_t orb2=0;orb2<norb;orb2++) {
							size_t gamma2 = orb2+spin1*norb; // spin2 == spin1 here
							matrix(p+gamma1*volume,k+gamma2*volume)=
								mp_.hoppings[orb1+orb2*norb+norb*norb*dir];
							matrix(k+gamma2*volume,p+gamma1*volume) = conj(matrix(p+gamma1*volume,k+gamma2*volume));
						}
					}
					
					for (size_t spin2=0;spin2<2;spin2++) {
						if (spin1==spin2) continue; // diagonal term already taken into account
						size_t gamma2 = orb1+spin2*norb; // orb2 == orb1 here
						matrix(p+gamma1*volume,p + gamma2*volume)=jmatrix[spin1+2*spin2];
						matrix(p + gamma2*volume,p+gamma1*volume) =
								conj(matrix(p+gamma1*volume,p + gamma2*volume));
					}
				}
			}
			threeOrbitalTerms_(matrix);
		}

		void auxCreateJmatrix(std::vector<ComplexType>& jmatrix,const
				typename DynVarsType::SpinType& dynVars,size_t site) const
		{
			if (!mp_.modulus[site]) {
				for (size_t i=0;i<jmatrix.size();i++) jmatrix[i] = 0;
				return;
			}
			
			jmatrix[0]=cos(dynVars.theta[site]);
		
			jmatrix[1]=ComplexType(sin(dynVars.theta[site])*cos(dynVars.phi[site]),
				sin(dynVars.theta[site])*sin(dynVars.phi[site]));
		
			jmatrix[2]=conj(jmatrix[1]);
		
			jmatrix[3]= -cos(dynVars.theta[site]);
		
			for (size_t i=0;i<jmatrix.size();i++) jmatrix[i] *= mp_.J;
		}

		size_t getSiteAtLayer(size_t xOrY,size_t layer,size_t dir) const
		{
			if (dir==0) { // x-dir: layer==0 is x=0, layer==1 is x=1
				return geometry_.coorToIndex(layer,xOrY);
			}
			// y-dir, layer==0 is y=0, layer==1 is y=1
			return geometry_.coorToIndex(xOrY,layer);
		}

		template<typename GreenFunctionType>
		void calcVelocitySquared(const GreenFunctionType& gf,
				PsimagLite::Matrix<FieldType>& v,size_t dir) const
		{
			size_t ly = geometry_.length();
			size_t norb = mp_.numberOfOrbitals;
			size_t volume = geometry_.volume();
			PsimagLite::Matrix<ComplexType> w(v.n_row(),v.n_col());
			
			for (size_t y=0;y<ly;y++) {
				for (size_t spin=0;spin<2;spin++) {
					for (size_t orb1=0;orb1<norb;orb1++) {
						size_t gamma1 = orb1 + spin*norb;
						size_t i = getSiteAtLayer(y,0,dir) + gamma1*volume; // x=0;

						for (size_t y2=0;y2<ly;y2++) {
							for (size_t orb2=0;orb2<norb;orb2++) {
								size_t gamma2 = orb2 + spin*norb;
								size_t j = getSiteAtLayer(y2,1,dir) +
								                gamma2*volume; // x=1;
								size_t dir2 = geometry_.getDirection(i,j);
								size_t h = orb1+orb2*norb+norb*norb*dir2;
								FieldType hopping = mp_.hoppings[h];
								if (fabs(hopping)<1e-8) continue;
								//std::cerr<<"dir="<<dir<<" isite="<<(i-gamma1*volume)<<" jsite="<<(j-gamma2*volume)<<" dir2="<<dir2<<" h="<<hopping<<"\n";
								for (size_t a=0;a<v.n_row();a++) {
									for (size_t b=0;b<v.n_col();b++) {
										ComplexType sum = velocity(gf,i,j,a,b)*
										       hopping;
										w(a,b) += sum;
									}
								}
							}
						}
					}
				}
			}
			
			for (size_t a=0;a<v.n_row();a++)
				for (size_t b=0;b<v.n_col();b++)  
					v(a,b) = std::real(std::conj(w(a,b))*w(a,b));
			
			FieldType sum = 0;
			for (size_t a=0;a<v.n_row();a++) sum += v(a,a);
			
			std::cerr<<"Trace for dir="<<dir<<" is "<<sum<<"\n";
			
		}

		
		template<typename GreenFunctionType>
		ComplexType velocity(const GreenFunctionType& gf,
				size_t i,
				size_t j,
				size_t a,
				size_t b) const
		{
			return std::conj(gf.matrix(i,a)) * gf.matrix(j,b) -
					std::conj(gf.matrix(j,a)) * gf.matrix(i,b);

		}

		const EngineParamsType& engineParams_;
		const ParametersModelType& mp_;
		const GeometryType& geometry_;
		DynVarsType dynVars_;
		size_t hilbertSize_;
		AdjustmentsType adjustments_;
		ProgressIndicatorType progress_;
		SpinOperationsType spinOperations_;
		ThreeOrbitalTermsType threeOrbitalTerms_;
		ObservablesStoredType observablesStored_;

	}; // PnictidesMultiOrbitals

	template<typename EngineParamsType,
	         typename ParametersModelType,
	         typename GeometryType,
	         typename ConcurrencyType>
	std::ostream& operator<<(std::ostream& os,
	                         const PnictidesMultiOrbitals<
	                           EngineParamsType,
	                           ParametersModelType,
	                           GeometryType,
	                           ConcurrencyType>& model)
	{
		os<<"ModelParameters\n";
		os<<model.mp_;
		return os;
	}
} // namespace Spf

/*@}*/
#endif // PNICTIDES_MULTI_ORBS_H
