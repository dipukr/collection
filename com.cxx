class ExpressionBinary : public Expression
{
	Expression *lhs;
	Expression *rhs;
	uint kind;
	Symbol sym;
public:	
	ExpressionBinary(Expression *lhs, Expression *rhs);
	void accept(ExpressionVisitor *visitor);
	Expression *lhs() const;
	Expression *rhs() const;
	bool lvalue() const;
};

class ExpressionCall : public Expression
{
	Expression *caller;
	List<Expression*> *arguments;
public:	
	ExpressionCall(Expression *lhs, Expression *rhs);
	void accept(ExpressionVisitor *visitor);
	Expression *lhs() const;
	Expression *rhs() const;
	bool lvalue() const;
};