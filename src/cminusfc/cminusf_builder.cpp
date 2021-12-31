#include "cminusf_builder.hpp"

#define CONST_FP(num) \
	ConstantFP::get((float)num, module.get())
#define CONST_INT(num) \
	ConstantInt::get(num, module.get())


// store temporary value
Value *tmp_val = nullptr;
// whether require lvalue
bool require_lvalue = false;
// function that is being built
Function *cur_fun = nullptr;
// detect scope pre-enter (for elegance only)
bool pre_enter_scope = false;

// types
Type *VOID_T;
Type *INT1_T;
Type *INT32_T;
Type *INT32PTR_T;
Type *FLOAT_T;
Type *FLOATPTR_T;

bool promote(IRBuilder *builder, Value **l_val_p, Value **r_val_p)
{
	bool is_integer;
	auto &l_val = *l_val_p;
	auto &r_val = *r_val_p;
	if (l_val->get_type() == r_val->get_type())
	{
		is_integer = l_val->get_type()->is_integer_type();
	}
	else
	{
		is_integer = false;
		if (l_val->get_type()->is_integer_type())
			l_val = builder->create_sitofp(l_val, FLOAT_T);
		else
			r_val = builder->create_sitofp(r_val, FLOAT_T);
	}
	return is_integer;
}


void CminusfBuilder::visit(ASTProgram &nd)
{
	VOID_T = Type::get_void_type(module.get());
	INT1_T = Type::get_int1_type(module.get());
	INT32_T = Type::get_int32_type(module.get());
	INT32PTR_T = Type::get_int32_ptr_type(module.get());
	FLOAT_T = Type::get_float_type(module.get());
	FLOATPTR_T = Type::get_float_ptr_type(module.get());

	for (auto decl : nd.declarations)
	{
		decl->accept(*this);
	}
}

void CminusfBuilder::visit(ASTNum &nd)
{
	if (nd.type == TYPE_INT)
		tmp_val = CONST_INT(nd.i_val);
	else
		tmp_val = CONST_FP(nd.f_val);
}

void CminusfBuilder::visit(ASTVarDeclaration &nd)
{
	Type *var_t;
	if (nd.type == TYPE_INT)
		var_t = Type::get_int32_type(module.get());
	else
		var_t = Type::get_float_type(module.get());
	if (nd.num == nullptr)
	{
		if (scope.in_global())
		{
			auto initializer = ConstantZero::get(var_t, module.get());
			auto var = GlobalVariable::create(
				nd.id,
				module.get(),
				var_t,
				false,
				initializer);
			scope.push(nd.id, var);
		}
		else
		{
			auto var = builder->create_alloca(var_t);
			scope.push(nd.id, var);
		}
	}
	else
	{
		auto *array_type = ArrayType::get(var_t, nd.num->i_val);
		if (scope.in_global())
		{
			auto initializer = ConstantZero::get(array_type, module.get());
			auto var = GlobalVariable::create(
				nd.id,
				module.get(),
				array_type,
				false,
				initializer);
			scope.push(nd.id, var);
		}
		else
		{
			auto var = builder->create_alloca(array_type);
			scope.push(nd.id, var);
		}
	}
}

void CminusfBuilder::visit(ASTFunDeclaration &nd)
{
	FunctionType *fun_type;
	Type *ret_tp;
	std::vector<Type *> param_types;
	if (nd.type == TYPE_INT)
		ret_tp = INT32_T;
	else if (nd.type == TYPE_FLOAT)
		ret_tp = FLOAT_T;
	else
		ret_tp = VOID_T;

	for (auto &param : nd.params)
	{
		if (param->type == TYPE_INT)
		{
			if (param->isarray)
			{
				param_types.push_back(INT32PTR_T);
			}
			else
			{
				param_types.push_back(INT32_T);
			}
		}
		else
		{
			if (param->isarray)
			{
				param_types.push_back(FLOATPTR_T);
			}
			else
			{
				param_types.push_back(FLOAT_T);
			}
		}
	}

	fun_type = FunctionType::get(ret_tp, param_types);
	auto fun =
		Function::create(
			fun_type,
			nd.id,
			module.get());
	scope.push(nd.id, fun);
	cur_fun = fun;
	auto funBB = BasicBlock::create(module.get(), "entry", fun);
	builder->set_insert_point(funBB);
	scope.enter();
	pre_enter_scope = true;
	std::vector<Value *> args;
	for (auto arg = fun->arg_begin(); arg != fun->arg_end(); arg++)
	{
		args.push_back(*arg);
	}
	for (int i = 0; i < nd.params.size(); ++i)
	{
		if (nd.params[i]->isarray)
		{
			Value *array_alloc;
			if (nd.params[i]->type == TYPE_INT)
				array_alloc = builder->create_alloca(INT32PTR_T);
			else
				array_alloc = builder->create_alloca(FLOATPTR_T);
			builder->create_store(args[i], array_alloc);
			scope.push(nd.params[i]->id, array_alloc);
		}
		else
		{
			Value *alloc;
			if (nd.params[i]->type == TYPE_INT)
				alloc = builder->create_alloca(INT32_T);
			else
				alloc = builder->create_alloca(FLOAT_T);
			builder->create_store(args[i], alloc);
			scope.push(nd.params[i]->id, alloc);
		}
	}
	nd.compound_stmt->accept(*this);
	if (builder->get_insert_block()->get_terminator() == nullptr)
	{
		if (cur_fun->get_return_type()->is_void_type())
			builder->create_void_ret();
		else if (cur_fun->get_return_type()->is_float_type())
			builder->create_ret(CONST_FP(0.));
		else
			builder->create_ret(CONST_INT(0));
	}
	scope.exit();
}

void CminusfBuilder::visit(ASTParam &nd) {}

void CminusfBuilder::visit(ASTCompoundStmt &nd)
{
	bool need_exit_scope = !pre_enter_scope;
	if (pre_enter_scope)
	{
		pre_enter_scope = false;
	}
	else
	{
		scope.enter();
	}

	for (auto &decl : nd.local_declarations)
	{
		decl->accept(*this);
	}

	for (auto &stmt : nd.statement_list)
	{
		stmt->accept(*this);
		if (builder->get_insert_block()->get_terminator() != nullptr)
			break;
	}

	if (need_exit_scope)
	{
		scope.exit();
	}
}

void CminusfBuilder::visit(ASTExpressionStmt &nd)
{
	if (nd.expression != nullptr)
		nd.expression->accept(*this);
}

void CminusfBuilder::visit(ASTSelectionStmt &nd)
{
	nd.expression->accept(*this);
	auto ret_val = tmp_val;
	auto trueBB = BasicBlock::create(module.get(), "", cur_fun);
	auto falseBB = BasicBlock::create(module.get(), "", cur_fun);
	auto contBB = BasicBlock::create(module.get(), "", cur_fun);
	Value *cond_val;
	if (ret_val->get_type()->is_integer_type())
		cond_val = builder->create_icmp_ne(ret_val, CONST_INT(0));
	else
		cond_val = builder->create_fcmp_ne(ret_val, CONST_FP(0.));

	if (nd.else_statement == nullptr)
	{
		builder->create_cond_br(cond_val, trueBB, contBB);
	}
	else
	{
		builder->create_cond_br(cond_val, trueBB, falseBB);
	}
	builder->set_insert_point(trueBB);
	nd.if_statement->accept(*this);

	if (builder->get_insert_block()->get_terminator() == nullptr)
		builder->create_br(contBB);

	if (nd.else_statement == nullptr)
	{
		falseBB->erase_from_parent();
	}
	else
	{
		builder->set_insert_point(falseBB);
		nd.else_statement->accept(*this);
		if (builder->get_insert_block()->get_terminator() == nullptr)
			builder->create_br(contBB);
	}

	builder->set_insert_point(contBB);
}

void CminusfBuilder::visit(ASTIterationStmt &nd)
{
	auto exprBB = BasicBlock::create(module.get(), "", cur_fun);
	if (builder->get_insert_block()->get_terminator() == nullptr)
		builder->create_br(exprBB);
	builder->set_insert_point(exprBB);
	nd.expression->accept(*this);
	auto ret_val = tmp_val;
	auto trueBB = BasicBlock::create(module.get(), "", cur_fun);
	auto contBB = BasicBlock::create(module.get(), "", cur_fun);
	Value *cond_val;
	if (ret_val->get_type()->is_integer_type())
		cond_val = builder->create_icmp_ne(ret_val, CONST_INT(0));
	else
		cond_val = builder->create_fcmp_ne(ret_val, CONST_FP(0.));

	builder->create_cond_br(cond_val, trueBB, contBB);
	builder->set_insert_point(trueBB);
	nd.statement->accept(*this);
	if (builder->get_insert_block()->get_terminator() == nullptr)
		builder->create_br(exprBB);
	builder->set_insert_point(contBB);
}

void CminusfBuilder::visit(ASTReturnStmt &nd)
{
	if (nd.expression == nullptr)
	{
		builder->create_void_ret();
	}
	else
	{
		auto fun_ret_tp = cur_fun->get_function_type()->get_return_type();
		nd.expression->accept(*this);
		if (fun_ret_tp != tmp_val->get_type())
		{
			if (fun_ret_tp->is_integer_type())
				tmp_val = builder->create_fptosi(tmp_val, INT32_T);
			else
				tmp_val = builder->create_sitofp(tmp_val, FLOAT_T);
		}

		builder->create_ret(tmp_val);
	}
}

void CminusfBuilder::visit(ASTVar &nd)
{
	auto var = scope.find(nd.id);
	assert(var != nullptr);
	auto is_integer = var->get_type()->get_pointer_element_type()->is_integer_type();
	auto is_float = var->get_type()->get_pointer_element_type()->is_float_type();
	auto is_ptr = var->get_type()->get_pointer_element_type()->is_pointer_type();
	bool should_return_lvalue = require_lvalue;
	require_lvalue = false;
	if (nd.expression == nullptr)
	{
		if (should_return_lvalue)
		{
			tmp_val = var;
			require_lvalue = false;
		}
		else
		{
			if (is_integer || is_float || is_ptr)
			{
				tmp_val = builder->create_load(var);
			}
			else
			{
				tmp_val = builder->create_gep(var, {CONST_INT(0), CONST_INT(0)});
			}
		}
	}
	else
	{
		nd.expression->accept(*this);
		auto val = tmp_val;
		Value *is_neg;
		auto exceptBB = BasicBlock::create(module.get(), "", cur_fun);
		auto contBB = BasicBlock::create(module.get(), "", cur_fun);
		if (val->get_type()->is_float_type())
			val = builder->create_fptosi(val, INT32_T);

		is_neg = builder->create_icmp_lt(val, CONST_INT(0));

		builder->create_cond_br(is_neg, exceptBB, contBB);
		builder->set_insert_point(exceptBB);
		auto neg_idx_except_fun = scope.find("neg_idx_except");
		builder->create_call(static_cast<Function *>(neg_idx_except_fun), {});
		if (cur_fun->get_return_type()->is_void_type())
			builder->create_void_ret();
		else if (cur_fun->get_return_type()->is_float_type())
			builder->create_ret(CONST_FP(0.));
		else
			builder->create_ret(CONST_INT(0));

		builder->set_insert_point(contBB);
		Value *tmp_ptr;
		if (is_integer || is_float)
			tmp_ptr = builder->create_gep(var, {val});
		else if (is_ptr)
		{
			auto array_load = builder->create_load(var);
			tmp_ptr = builder->create_gep(array_load, {val});
		}
		else
			tmp_ptr = builder->create_gep(var, {CONST_INT(0), val});
		if (should_return_lvalue)
		{
			tmp_val = tmp_ptr;
			require_lvalue = false;
		}
		else
		{
			tmp_val = builder->create_load(tmp_ptr);
		}
	}
}

void CminusfBuilder::visit(ASTAssignExpression &nd)
{
	nd.expression->accept(*this);
	auto expr_result = tmp_val;
	require_lvalue = true;
	nd.var->accept(*this);
	auto var_addr = tmp_val;
	if (var_addr->get_type()->get_pointer_element_type() != expr_result->get_type())
	{
		if (expr_result->get_type() == INT32_T)
			expr_result = builder->create_sitofp(expr_result, FLOAT_T);
		else
			expr_result = builder->create_fptosi(expr_result, INT32_T);
	}
	builder->create_store(expr_result, var_addr);
	tmp_val = expr_result;
}

void CminusfBuilder::visit(ASTSimpleExpression &nd)
{
	if (nd.additive_expression_r == nullptr)
	{
		nd.additive_expression_l->accept(*this);
	}
	else
	{
		nd.additive_expression_l->accept(*this);
		auto l_val = tmp_val;
		nd.additive_expression_r->accept(*this);
		auto r_val = tmp_val;
		bool is_integer = promote(builder, &l_val, &r_val);
		Value *cmp;
		switch (nd.op)
		{
		case OP_LT:
			if (is_integer)
				cmp = builder->create_icmp_lt(l_val, r_val);
			else
				cmp = builder->create_fcmp_lt(l_val, r_val);
			break;
		case OP_LE:
			if (is_integer)
				cmp = builder->create_icmp_le(l_val, r_val);
			else
				cmp = builder->create_fcmp_le(l_val, r_val);
			break;
		case OP_GE:
			if (is_integer)
				cmp = builder->create_icmp_ge(l_val, r_val);
			else
				cmp = builder->create_fcmp_ge(l_val, r_val);
			break;
		case OP_GT:
			if (is_integer)
				cmp = builder->create_icmp_gt(l_val, r_val);
			else
				cmp = builder->create_fcmp_gt(l_val, r_val);
			break;
		case OP_EQ:
			if (is_integer)
				cmp = builder->create_icmp_eq(l_val, r_val);
			else
				cmp = builder->create_fcmp_eq(l_val, r_val);
			break;
		case OP_NEQ:
			if (is_integer)
				cmp = builder->create_icmp_ne(l_val, r_val);
			else
				cmp = builder->create_fcmp_ne(l_val, r_val);
			break;
		}

		tmp_val = builder->create_zext(cmp, INT32_T);
	}
}

void CminusfBuilder::visit(ASTAdditiveExpression &nd)
{
	if (nd.additive_expression == nullptr)
	{
		nd.term->accept(*this);
	}
	else
	{
		nd.additive_expression->accept(*this);
		auto l_val = tmp_val;
		nd.term->accept(*this);
		auto r_val = tmp_val;
		bool is_integer = promote(builder, &l_val, &r_val);
		switch (nd.op)
		{
		case OP_PLUS:
			if (is_integer)
				tmp_val = builder->create_iadd(l_val, r_val);
			else
				tmp_val = builder->create_fadd(l_val, r_val);
			break;
		case OP_MINUS:
			if (is_integer)
				tmp_val = builder->create_isub(l_val, r_val);
			else
				tmp_val = builder->create_fsub(l_val, r_val);
			break;
		}
	}
}

void CminusfBuilder::visit(ASTTerm &nd)
{
	if (nd.term == nullptr)
	{
		nd.factor->accept(*this);
	}
	else
	{
		nd.term->accept(*this);
		auto l_val = tmp_val;
		nd.factor->accept(*this);
		auto r_val = tmp_val;
		bool is_integer = promote(builder, &l_val, &r_val);
		switch (nd.op)
		{
		case OP_MUL:
			if (is_integer)
				tmp_val = builder->create_imul(l_val, r_val);
			else
				tmp_val = builder->create_fmul(l_val, r_val);
			break;
		case OP_DIV:
			if (is_integer)
				tmp_val = builder->create_isdiv(l_val, r_val);
			else
				tmp_val = builder->create_fdiv(l_val, r_val);
			break;
		}
	}
}

void CminusfBuilder::visit(ASTCall &nd)
{
	auto fun = static_cast<Function *>(scope.find(nd.id));
	std::vector<Value *> args;
	auto param_type = fun->get_function_type()->param_begin();
	for (auto &arg : nd.args)
	{
		arg->accept(*this);
		if (!tmp_val->get_type()->is_pointer_type() &&
			*param_type != tmp_val->get_type())
		{
			if (tmp_val->get_type()->is_integer_type())
				tmp_val = builder->create_sitofp(tmp_val, FLOAT_T);
			else
				tmp_val = builder->create_fptosi(tmp_val, INT32_T);
		}
		args.push_back(tmp_val);
		param_type++;
	}

	tmp_val = builder->create_call(static_cast<Function *>(fun), args);
}



