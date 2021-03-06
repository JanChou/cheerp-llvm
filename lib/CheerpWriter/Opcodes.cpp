//===-- Opcodes.cpp - The Cheerp JavaScript generator ---------------------===//
//
//                     Cheerp: The C++ compiler for the Web
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Copyright 2014-2015 Leaning Technologies
//
//===----------------------------------------------------------------------===//

#include "llvm/Cheerp/Utility.h"
#include "llvm/Cheerp/Writer.h"

using namespace llvm;
using namespace cheerp;

void CheerpWriter::compileIntegerComparison(const llvm::Value* lhs, const llvm::Value* rhs, CmpInst::Predicate p)
{
	if(lhs->getType()->isPointerTy())
	{
		if(p==CmpInst::ICMP_EQ || p==CmpInst::ICMP_NE)
			compileEqualPointersComparison(lhs, rhs, p);
		else
		{
			//Comparison on different bases is anyway undefined, so ignore them
			compilePointerOffset( lhs );
			compilePredicate(p);
			compilePointerOffset( rhs );
		}
	}
	else
	{
		compileOperandForIntegerPredicate(lhs,p);
		compilePredicate(p);
		compileOperandForIntegerPredicate(rhs,p);
	}
}

void CheerpWriter::compilePtrToInt(const llvm::Value* v)
{
	stream << '(';
	compilePointerOffset(v);
	stream << ')';
}

void CheerpWriter::compileSubtraction(const llvm::Value* lhs, const llvm::Value* rhs)
{
	//Integer subtraction
	//TODO: optimize negation
	stream << "((";
	compileOperand(lhs);
	stream << '-';
	compileOperand(rhs);
	stream << ')';
	if(types.isI32Type(lhs->getType()))
		stream << ">>0";
	else
		stream << '&' << getMaskForBitWidth(lhs->getType()->getIntegerBitWidth());
	stream << ')';
}

void CheerpWriter::compileBitCast(const llvm::User* bc_inst, POINTER_KIND kind)
{
	if(kind==COMPLETE_OBJECT)
		compileCompleteObject(bc_inst->getOperand(0));
	else
	{
		if(PA.getConstantOffsetForPointer(bc_inst))
			compilePointerBase(bc_inst);
		else if(PA.getPointerKind(bc_inst->getOperand(0)) == REGULAR && !isa<Argument>(bc_inst->getOperand(0)))
			compileOperand(bc_inst->getOperand(0));
		else
		{
			stream << "{d:";
			compilePointerBase(bc_inst, true);
			stream << ",o:";
			compilePointerOffset(bc_inst, true);
			stream << "}";
		}
	}
}

void CheerpWriter::compileBitCastBase(const llvm::User* bi, bool forEscapingPointer)
{
	Type* src=bi->getOperand(0)->getType();
	Type* dst=bi->getType();
	//Special case unions
	if(TypeSupport::hasByteLayout(src->getPointerElementType()) && forEscapingPointer)
	{
		//Find the type
		llvm::Type* elementType = dst->getPointerElementType();
		bool isArray=isa<ArrayType>(elementType);
		llvm::Type* pointedType = (isArray)?elementType->getSequentialElementType():elementType;
		if(TypeSupport::isTypedArrayType(pointedType, /* forceTypedArray*/ true))
		{
			stream << "new ";
			compileTypedArrayType(pointedType);
			stream << '(';
			compileCompleteObject(bi->getOperand(0));
			stream << ".buffer)";
			return;
		}
	}

	compilePointerBase(bi->getOperand(0), forEscapingPointer);
}

void CheerpWriter::compileBitCastOffset(const llvm::User* bi)
{
	Type* src=bi->getOperand(0)->getType();
	Type* dst=bi->getType();
	//Special case unions
	if(TypeSupport::hasByteLayout(src->getPointerElementType()))
	{
		//Find the type
		llvm::Type* elementType = dst->getPointerElementType();
		bool isArray=isa<ArrayType>(elementType);
		llvm::Type* pointedType = (isArray)?elementType->getSequentialElementType():elementType;
		if(TypeSupport::isTypedArrayType(pointedType, /* forceTypedArray*/ true))
		{
			stream << '0';
			return;
		}
	}

	compilePointerOffset(bi->getOperand(0));
}

