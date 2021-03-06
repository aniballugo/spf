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

/*! \file DmsMultiOrbitalFields.h
 *
 *  Wrapper around classical spin, to say that only spin is a MC variable
 *
 */
#ifndef DMS_MULTI_ORB_FIELDS_H
#define DMS_MULTI_ORB_FIELDS_H
#include "Spin.h"
#include "SpinOperations.h"

namespace Spf {
	template<typename FieldType,typename GeometryType>
	class DmsMultiOrbitalFields {
	public:	
		typedef Spin<FieldType> SpinType;
		typedef ClassicalSpinOperations<GeometryType,SpinType> SpinOperationsType;
		typedef SpinType Type0;
		typedef SpinType Type1; //bogus
		typedef SpinOperationsType OperationsType0;
		typedef SpinOperationsType OperationsType1; // bogus
		
		DmsMultiOrbitalFields(size_t vol,const std::string& mcstarttype) :
				spin_(vol,mcstarttype)
		{}
		
		//! only spin for this model needs MC simulation
		size_t size() const { return 1; }
		
		const std::string& name(size_t i) const { return name_; }
		
		SpinType& getField(SpinType*)
		{
			return spin_;
		}
		
		template<typename FieldType2,typename GeometryType2>
		friend std::ostream& operator<<(
				std::ostream& os,
				DmsMultiOrbitalFields<FieldType2,GeometryType2>& f);
		
	private:
		static const std::string name_;
		SpinType spin_;
		
	}; // DmsMultiOrbitalFields
	
	template<typename FieldType,typename GeometryType>
	std::ostream& operator<<(
			std::ostream& os,
			DmsMultiOrbitalFields<FieldType,GeometryType>& f)
	{
		os<<f.spin_;
		return os;
	}
	template<typename FieldType,typename GeometryType>
	const std::string DmsMultiOrbitalFields<FieldType,GeometryType>::name_="spin";
	
} // namespace Spf

/*@}*/
#endif //DMS_MULTI_ORB_FIELDS_H
