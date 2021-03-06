// BEGIN LICENSE BLOCK
/*
Copyright (c) 2009 , UT-Battelle, LLC
All rights reserved

[DMRG++, Version 2.0.0]
[by G.A., Oak Ridge National Laboratory]

UT Battelle Open Source Software License 11242008

OPEN SOURCE LICENSE

Subject to the conditions of this License, each
contributor to this software hereby grants, free of
charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), a
perpetual, worldwide, non-exclusive, no-charge,
royalty-free, irrevocable copyright license to use, copy,
modify, merge, publish, distribute, and/or sublicense
copies of the Software.

1. Redistributions of Software must retain the above
copyright and license notices, this list of conditions,
and the following disclaimer.  Changes or modifications
to, or derivative works of, the Software should be noted
with comments and the contributor and organization's
name.

2. Neither the names of UT-Battelle, LLC or the
Department of Energy nor the names of the Software
contributors may be used to endorse or promote products
derived from this software without specific prior written
permission of UT-Battelle.

3. The software and the end-user documentation included
with the redistribution, with or without modification,
must include the following acknowledgment:

"This product includes software produced by UT-Battelle,
LLC under Contract No. DE-AC05-00OR22725  with the
Department of Energy."
 
*********************************************************
DISCLAIMER

THE SOFTWARE IS SUPPLIED BY THE COPYRIGHT HOLDERS AND
CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
COPYRIGHT OWNER, CONTRIBUTORS, UNITED STATES GOVERNMENT,
OR THE UNITED STATES DEPARTMENT OF ENERGY BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

NEITHER THE UNITED STATES GOVERNMENT, NOR THE UNITED
STATES DEPARTMENT OF ENERGY, NOR THE COPYRIGHT OWNER, NOR
ANY OF THEIR EMPLOYEES, REPRESENTS THAT THE USE OF ANY
INFORMATION, DATA, APPARATUS, PRODUCT, OR PROCESS
DISCLOSED WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS.

*********************************************************


*/
// END LICENSE BLOCK
/** \ingroup SPF */
/*@{*/

/*! \file ParametersPnictidesTwoOrbitals.h
 *
 *  Contains the parameters for thePnictidesTwoOrbitals model
 *
 */
#ifndef PARAMETERSPNICTIDESTWOORBITALS_H
#define PARAMETERSPNICTIDESTWOORBITALS_H

namespace Spf {
	//!Model Parameters, please don't add member functions, this is a struct
	template<typename ParametersEngineType,typename IoInType>
	struct ParametersPnictidesTwoOrbitals {
		typedef typename ParametersEngineType::FieldType RealType;

		static size_t const numberOfOrbitals = 2;

		ParametersPnictidesTwoOrbitals(
				IoInType& io,
				const ParametersEngineType& engineParams)
		{
			io.read(hoppings,"Hoppings");

			io.readline(J,"CouplingJ=");
			io.read(potentialV,"PotentialV");
			try {
				RealType x = 0;
				io.readline(x,"JAFNN=");
				jafNn.resize(2,x);
			} catch (std::exception& e) {
				// must rewind because exception consumed file:
				io.rewind();
				io.read(jafNn,"JAFNN");
			}

			io.readline(jafNnn,"JAFNNN=");
			io.readline(magneticField,"MagneticField=");

			modulus.resize(potentialV.size());
			if (engineParams.options.find("hasModulus")==std::string::npos
					&&
					engineParams.options.find("hasmodulus")==std::string::npos) {
				for (size_t i=0;i<modulus.size();i++)
					modulus[i] = 1;
				return;
			}

			for (size_t i=0;i<modulus.size();i++) modulus[i] = 0;

			std::vector<size_t> tmp;
			io.read(tmp,"Modulus");
			for (size_t i=0;i<tmp.size();i++) modulus[tmp[i]] = 1;

		}

		// packed as orbital1+orbital2*2 + dir*4
		// where dir=0 is x, dir=1 is y, dir=2 is x+y and dir=3 is x-y
		std::vector<RealType> hoppings;
		// J value
		RealType J;
		// Onsite potential values, one for each site
		std::vector<RealType> potentialV;
		
		// target number of electrons  in the system
		//int nOfElectrons;
		
		// JAF n-n
		std::vector<RealType> jafNn;
		
		// JAF n-n-n
		RealType jafNnn;

		// Magnetic Field B for Zeeman term
		RealType magneticField;

		// moduli of the classical spins (either 0 or 1)
		std::vector<size_t> modulus;
	};


	
	//! Function that prints model parameters to stream os
	template<typename ParametersEngineType,typename IoInType>
	std::ostream& operator<<(
		std::ostream &os,
		const ParametersPnictidesTwoOrbitals<ParametersEngineType,IoInType>& parameters)
	{
		os<<"parameters.jafNn="<<parameters.jafNn;
		os<<"parameters.jafNnn="<<parameters.jafNnn<<"\n";
		os<<"parameters.J="<<parameters.J<<"\n";
		os<<"parameters.magneticField="<<parameters.magneticField<<"\n";
		os<<"parameters.potentialV\n";
		os<<parameters.potentialV;
		os<<"parameters.hoppings\n";
		os<<parameters.hoppings;
		os<<"parameters.modulus=";
		for (size_t i=0;i<parameters.modulus.size();i++) {
			if (parameters.modulus[i]) os<<i<<" ";
		}
		os<<"\n";
		return os;
	}
} // namespace Spf

/*@}*/
#endif
