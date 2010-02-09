
/** \ingroup SPF */
/*@{*/

/*! \file ClassicalSpinOperations.h
 *
 *  Operations related to a vector of classical spins
 *
 */
#ifndef CLASSICAL_SPIN_OPS_H
#define CLASSICAL_SPIN_OPS_H
#include "Utils.h"

namespace Spf {
	
	template<typename GeometryType,typename DynVarsType>
	class ClassicalSpinOperations {
		typedef typename DynVarsType::FieldType FieldType;
			
		static const bool isingSpins_ = false; // FIXME: make it runtime option
		
	public:
		ClassicalSpinOperations(const GeometryType& geometry,FieldType mcwindow) 
			: geometry_(geometry),mcwindow_(mcwindow),dynVars2_(0,"none")
		{
		}
		
		void set(DynVarsType& dynVars)
		{
			dynVars_=&dynVars;
		}
		
		template<typename RandomNumberGeneratorType>
		void propose(size_t i,RandomNumberGeneratorType& rng)
		{
			FieldType thetaOld = dynVars_->theta[i];
			FieldType phiOld = dynVars_->phi[i];
			
			dynVars2_ = *dynVars_;
			
			propose_(thetaOld,phiOld,dynVars2_.theta[i],dynVars2_.phi[i],rng);
			
			
		}
		
		const DynVarsType& dynVars2() const { return dynVars2_; } 
		
		FieldType deltaDirect(size_t i,FieldType coupling1,FieldType coupling2) const
		{
			FieldType sum = dSDirect(*dynVars_,dynVars2_,i,coupling1);
			sum += directExchange2(dynVars2_,coupling2)
						-directExchange2(*dynVars_,coupling2);
			return sum;
		}
		
				
		FieldType sineUpdate(size_t i) const
		{
			FieldType sineupdate= sin(dynVars_->theta[i]);
			if (sineupdate!=0) {
				sineupdate = sin(dynVars2_.theta[i])/sineupdate;
			} else {
				sineupdate = 1.0;
			}
			return sineupdate;
		}
		
		void accept(size_t i)
		{
			dynVars_->theta[i]=dynVars2_.theta[i];
			dynVars_->phi[i]=dynVars2_.phi[i];
		}
		
		FieldType directExchange2(const DynVarsType& dynVars,FieldType coupling) const
		{
			size_t n = dynVars.theta.size();
			FieldType dS = 0;
			
			for (size_t i=0;i<n;i++) {
				for (size_t k = 0; k<geometry_.z(2); k++){
					size_t j=geometry_.neighbor(i,k,2).first; /**next nearest neighbor */
					FieldType t1=dynVars.theta[i];
					FieldType t2=dynVars.theta[j];
					FieldType p1=dynVars.phi[i];
					FieldType p2=dynVars.phi[j];
					FieldType tmp = cos(t1)*cos(t2)+sin(t1)*sin(t2)*(cos(p1)*cos(p2)+sin(p1)*sin(p2));
					dS += coupling*tmp; 
				}
			}
			
			return dS*0.5;
		}
		
	private:
		
		template<typename RandomNumberGeneratorType>
		void propose_(FieldType thetaOld, FieldType phiOld, FieldType &thetaNew,FieldType &phiNew,RandomNumberGeneratorType& rng)
		{
			if (isingSpins_) {
				if (thetaOld==0) thetaNew=M_PI; 
				else thetaNew=0;
				phiNew=0;
			} else {
				if (mcwindow_<0) {
					thetaNew = 2*rng()-1;
					phiNew = 2*M_PI*rng();
					thetaNew = acos(thetaNew);
				} else {
					thetaNew=2*rng()- 1;
					if (thetaNew < -1) thetaNew= 0;
					if (thetaNew > 1) thetaNew = 0;		
					phiNew=phiOld+2*M_PI*(rng()- 0.5)*mcwindow_;
					thetaNew = acos(thetaNew);
				}
				/*if (ether.isSet("sineupdate")) {
					thetaNew = M_PI*ether.rng.myRandom();
				}*/
			
				while (thetaNew<0) {
					thetaNew = -thetaNew;
					phiNew+=M_PI;
				}	
				while (thetaNew>M_PI) {
					thetaNew -= M_PI;
					phiNew+=M_PI;
				}
					
				while (phiNew<0) phiNew += 2*M_PI;
				while (phiNew>2*M_PI) phiNew -= 2*M_PI;
			}
		}
		
		FieldType dSDirect(const DynVarsType& dynVars,const DynVarsType& dynVars2, size_t i,FieldType coupling) const
		{
			FieldType dS = 0;
				
			for (size_t k = 0; k<geometry_.z(1); k++){
				size_t j=geometry_.neighbor(i,k).first;
				FieldType tmp = (sin(dynVars2.theta[i])*cos(dynVars2.phi[i])-sin(dynVars.theta[i])*
					cos(dynVars.phi[i]))*sin(dynVars.theta[j])*cos(dynVars.phi[j]) +
						(sin(dynVars2.theta[i])*sin(dynVars2.phi[i])-sin(dynVars.theta[i])*
					sin(dynVars.phi[i]))*sin(dynVars.theta[j])*sin(dynVars.phi[j]) +
					(cos(dynVars2.theta[i])-cos(dynVars.theta[i]))*cos(dynVars.theta[j]);
				dS += coupling*tmp;
			}
			//if (ether.isSet("magneticfield")) tmp = Zeeman(dynVars2,geometry,ether)-Zeeman(dynVars,geometry,ether);
			//else tmp =0;
			// dS += tmp;
		
			return dS;
		}
		
		const GeometryType& geometry_;
		const FieldType& mcwindow_;
		DynVarsType* dynVars_;
		DynVarsType dynVars2_;
		
		
	}; // Engine
} // namespace Spf

/*@}*/
#endif // CLASSICAL_SPIN_OPS_H