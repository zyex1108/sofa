/******************************************************************************
*       SOFA, Simulation Open-Framework Architecture, version 1.0 beta 4      *
*                (c) 2006-2009 MGH, INRIA, USTL, UJF, CNRS                    *
*                                                                             *
* This library is free software; you can redistribute it and/or modify it     *
* under the terms of the GNU Lesser General Public License as published by    *
* the Free Software Foundation; either version 2.1 of the License, or (at     *
* your option) any later version.                                             *
*                                                                             *
* This library is distributed in the hope that it will be useful, but WITHOUT *
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or       *
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License *
* for more details.                                                           *
*                                                                             *
* You should have received a copy of the GNU Lesser General Public License    *
* along with this library; if not, write to the Free Software Foundation,     *
* Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.          *
*******************************************************************************
*                               SOFA :: Modules                               *
*                                                                             *
* Authors: The SOFA Team and external contributors (see Authors.txt)          *
*                                                                             *
* Contact information: contact@sofa-framework.org                             *
******************************************************************************/
#include "OpenCLTypes.h"
#include "OpenCLFixedConstraint.inl"
#include <sofa/core/ObjectFactory.h>
#include <sofa/defaulttype/Vec3Types.h>
#include <sofa/defaulttype/RigidTypes.h>

#include "myopencl.h"
#include "OpenCLProgram.h"
#include "OpenCLKernel.h"

#define DEBUG_TEXT(t) //printf("\t%s\t %s %d\n",t,__FILE__,__LINE__);

#define BSIZE 16

namespace sofa
{

namespace gpu
{

namespace opencl
{


SOFA_DECL_CLASS(OpenCLFixedConstraint)

int FixedConstraintOpenCLClass = core::RegisterObject("Supports GPU-side computations using OPENCL")
        .add< component::constraint::FixedConstraint<OpenCLVec3fTypes> >()
        .add< component::constraint::FixedConstraint<OpenCLVec3f1Types> >()
#ifdef SOFA_DEV
        .add< component::constraint::FixedConstraint<OpenCLRigid3fTypes> >()
#endif // SOFA_DEV
        .add< component::constraint::FixedConstraint<OpenCLVec3dTypes> >()
        .add< component::constraint::FixedConstraint<OpenCLVec3d1Types> >()
#ifdef SOFA_DEV
        .add< component::constraint::FixedConstraint<OpenCLRigid3dTypes> >()
#endif // SOFA_DEV
        ;





////////////////////////////////////
//////   kernels


sofa::helper::OpenCLProgram* FixedConstraintOpenCLFloat_program;
sofa::helper::OpenCLProgram* FixedConstraintOpenCLDouble_program;


void FixedConstraint_CreateProgramWithFloat()
{
    if(FixedConstraintOpenCLFloat_program==NULL)
    {

        std::map<std::string, std::string> types;
        types["Real"]="float";
        types["Real4"]="float4";

        FixedConstraintOpenCLFloat_program
            = new sofa::helper::OpenCLProgram(sofa::helper::OpenCLProgram::loadSource("OpenCLFixedConstraint.cl"),&types);

        FixedConstraintOpenCLFloat_program->buildProgram();
        sofa::gpu::opencl::myopenclShowError(__FILE__,__LINE__);
        std::cout << FixedConstraintOpenCLFloat_program->buildLog(0);
    }
}

void FixedConstraint_CreateProgramWithDouble()
{

    if(FixedConstraintOpenCLDouble_program==NULL)
    {

        std::map<std::string, std::string> types;
        types["Real"]="double";
        types["Real4"]="double4";

        FixedConstraintOpenCLDouble_program
            = new sofa::helper::OpenCLProgram(sofa::helper::OpenCLProgram::loadSource("OpenCLFixedConstraint.cl"),&types);

        FixedConstraintOpenCLDouble_program->buildProgram();

    }
}


sofa::helper::OpenCLKernel * FixedConstraintOpenCL3f_addForce_kernel;
void FixedConstraintOpenCL3f_projectResponseIndexed(unsigned int size, const _device_pointer indices, _device_pointer dx)
{
    DEBUG_TEXT("FixedConstraintOpenCL3f_projectResponseIndexed");
    BARRIER(dx,__FILE__,__LINE__);

    FixedConstraint_CreateProgramWithFloat();
    if(FixedConstraintOpenCL3f_addForce_kernel==NULL)FixedConstraintOpenCL3f_addForce_kernel
            = new sofa::helper::OpenCLKernel(FixedConstraintOpenCLFloat_program,"FixedConstraint3t_projectResponseIndexed");


    FixedConstraintOpenCL3f_addForce_kernel->setArg<unsigned int>(0,&size);
    FixedConstraintOpenCL3f_addForce_kernel->setArg<_device_pointer>(1,&indices);
    FixedConstraintOpenCL3f_addForce_kernel->setArg<_device_pointer>(2,&dx);


    size_t local_size[1];
    local_size[0]=BSIZE;

    size_t work_size[1];
    work_size[0]=((size%BSIZE)==0)?size:BSIZE*(size/BSIZE+1);

    //std::cout << __LINE__ << __FILE__ << " " << size << " " << nbSpringPerVertex << " " << springs.offset << " " << f.offset << " " <<  x.offset << " " <<  v.offset<< " " << dfdx.offset << "\n";
    //std::cout << local_size[0] << " " << size << " " <<work_size[0] << "\n";

    FixedConstraintOpenCL3f_addForce_kernel->execute(0,1,NULL,work_size,local_size);	//note: num_device = const = 0

    BARRIER(dx,__FILE__,__LINE__);
    DEBUG_TEXT("~FixedConstraintOpenCL3f_projectResponseIndexed");
}


void FixedConstraintOpenCL3f_projectResponseContiguous(unsigned int size, _device_pointer dx)
{
    DEBUG_TEXT("FixedConstraintOpenCL3f_projectResponseContiguous");
    BARRIER(dx,__FILE__,__LINE__);

    OpenCLMemoryManager<float>::memsetDevice(0,dx,0,size*3*sizeof(float));
//	NOT_IMPLEMENTED()

    BARRIER(dx,__FILE__,__LINE__);
    DEBUG_TEXT("~FixedConstraintOpenCL3f_projectResponseContiguous");
}



void FixedConstraintOpenCL3f1_projectResponseContiguous(unsigned int /*size*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
void FixedConstraintOpenCL3f1_projectResponseIndexed(unsigned int /*size*/, const _device_pointer /*indices*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
#ifdef SOFA_DEV
void FixedConstraintOpenCLRigid3f_projectResponseContiguous(unsigned int /*size*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
void FixedConstraintOpenCLRigid3f_projectResponseIndexed(unsigned int /*size*/, const _device_pointer /*indice*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
#endif // SOFA_DEV



void FixedConstraintOpenCL3d_projectResponseContiguous(unsigned int /*size*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
void FixedConstraintOpenCL3d_projectResponseIndexed(unsigned int /*size*/, const _device_pointer /*indices*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
void FixedConstraintOpenCL3d1_projectResponseContiguous(unsigned int /*size*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
void FixedConstraintOpenCL3d1_projectResponseIndexed(unsigned int /*size*/, const _device_pointer /*indices*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
#ifdef SOFA_DEV
void FixedConstraintOpenCLRigid3d_projectResponseContiguous(unsigned int /*size*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
void FixedConstraintOpenCLRigid3d_projectResponseIndexed(unsigned int /*size*/, const _device_pointer /*indice*/, _device_pointer /*dx*/) {NOT_IMPLEMENTED();}
#endif // SOFA_DEV






} // namespace opencl

} // namespace gpu

} // namespace sofa

#undef BSIZE
