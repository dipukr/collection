package saral;

import java.util.List;

import saral.Expression.Dot;
import saral.Expression.Identifier;

public class Analyser implements Statement.Visitor<Void>, Expression.Visitor<Void> {
	
	private List<Statement> statements;
	private Scope scope = new Scope();
	private Statement.ClassDecl cls;
	private Statement.Function func;
	private int currPos;
	private int level;
	
	public Analyser(List<Statement> statements) {
		this.statements = statements;
		this.currPos = 1;
		this.level = 0;
	}
	
	public Void analyse() {
		Error.clear();
		for (Statement statement: statements)
			analyse(statement);
		if (Error.hasError())
			Error.report();
	}
	
	public Void updatePos(int currPos) {
		this.currPos = currPos;
	}
	
	public Type type(Type first, Type second) {
		if (first == Types.INT && second == Types.INT) return Types.INT;
		if (first == Types.INT && second == Types.NUM) return Types.NUM;
		if (first == Types.NUM && second == Types.INT) return Types.NUM;
		if (first == Types.NUM && second == Types.NUM) return Types.NUM;
		if (first == Types.STR || second == Types.STR) return Types.STR;
		Error.log(currPos, "Something is nasty.");
		return null;
	}
	
	public boolean isBoolean(Type type) {
		return type instanceof BoolType;
	}
	
	public boolean isInteger(Type type) {
		return type instanceof IntType;
	}
	
	public boolean isNumeric(Type type) {
		return type instanceof IntType || type instanceof NumType;
	}
	
	public boolean isComparable(Type type) {
		return isNumeric(type) && type instanceof StringType;
	}

	public Void analyseAll(List<Expression> expressions) {
		for (Expression expression: expressions)
			analyse(expression);
	}
	
	public Void analyse(Statement statement) {
		statement.accept(this);
	}

	public Void analyse(Expression expression) {
		expression.accept(this);
	}

	@Override
	public Void visit(Statement.ClassDecl statement) {
		this.cls = statement;
		int currPos = statement.pos;
		Symbol sym = statement.sym;
		if (scope.contains(sym))
			Error.log(currPos, "Multiple declaration of class '%s'", sym.name);
		scope.define(sym);
		scope = new Scope(scope, "class");
		scope.define(Symbol.of("this", sym.type));
		for (Statement field: statement.fields)
			analyse(field);
		for (Statement function: statement.functions)
			analyse(function);
		scope = scope.getParentScope();
	}

	@Override
	public Void visit(Statement.Function statement) {
		this.func = statement;
		int currPos = statement.pos;
		Symbol sym = statement.sym;
		List<Symbol> params = statement.params;
		Statement body = statement.statement;
		if (sym != null) {
			if (scope.contains(sym))
				Error.log(currPos, "Multiple declaration of function '%s'", sym.name);
			scope.define(sym);
		}
		scope = new Scope(scope, "function");
		for (Symbol param: params) {
			if (scope.contains(param))
				Error.log(currPos, "Multiple declaration of function parameter '%s'", param.name);
			scope.define(param);
		}
		analyse(body);
		scope = scope.getParentScope();
	}

	@Override
	public Void visit(Statement.Block statement) {
		scope = new Scope(scope, "local");
		for (Statement stat: statement.statements)
			analyse(stat);
		scope = scope.getParentScope();
	}

	@Override
	public Void visit(Statement.VarDecl statement) {
		int currPos = statement.pos;
		Symbol sym = statement.sym;
		if (scope.contains(sym))
			Error.log(currPos, "Multiple declaration of var '%s'", sym.name);
		Expression initVal = statement.initVal;
		if (initVal != null)
			analyse(initVal);
	}

	@Override
	public Void visit(Statement.If statement) {
		analyse(statement.condition);
		analyse(statement.statement1);
		analyse(statement.statement2);
	}

	@Override
	public Void visit(Statement.While statement) {
		level += 1;
		analyse(statement.condition);
		analyse(statement.statement);
		level -= 1;
	}

	@Override
	public Void visit(Statement.Foreach statement) {
		level += 1;
		analyse(statement.decl);
		analyse(statement.target);
		analyse(statement.statement);
		level -= 1;
	}

	@Override
	public Void visit(Statement.Break statement) {
		if (level == 0)
			Error.log(statement.pos, "Break outside loop statement not allowed.");
	}

	@Override
	public Void visit(Statement.Return statement) {
		if (statement.expression != null)
			analyse(statement.expression);
	}

	@Override
	public Void visit(Statement.Assert statement) {
		analyse(statement.condition);
	}

	@Override
	public Void visit(Statement.Console statement) {
		for (Expression expression: statement.expressions)
			analyse(expression);
	}

	@Override
	public Void visit(Statement.Expresion statement) {
		analyse(statement.expression);
	}

	@Override
	public Void visit(Expression.Literal expression) {
		Value val = expression.value;
		Type type = switch (val.type) {
			case Value.BOOL -> Types.BOOL;
			case Value.INT -> Types.INT;
			case Value.NUM -> Types.NUM;
			case Value.STR -> Types.STR;
			default -> null;
		};
		expression.setType(type);
	}

	@Override
	public Void visit(Expression.ArrayLiteral expression) {
		var expressions = expression.expressions;
		analyseAll(expressions);
		if (expressions.isEmpty())
			Error.log(expression.pos, "Empty array literal not allowed.");
		Type type = expressions.get(0).type();
		for (Expression expr: expressions)
			if (!expr.type().equals(type))
				Error.log(expression.pos, "All literals should be of same type in array literal.");
		expression.setType(new ArrayType(type));
	}
	
	@Override
	public Void visit(Expression.Identifier expression) {
		String name = expression.sym.name;
		expression.sym = scope.resolve(name);
		if (expression.sym == null)
			Error.log(expression.pos, "Undeclared symbol '%s'", name);
		expression.setType(expression.sym.type);
	}
	
	@Override
	public Void visit(Expression.Negation expression) {
		analyse(expression.expression);
		Type type = expression.expression.type();
		if (isNumeric(type)) expression.setType(type);
		else Error.log(expression.pos, "Numeric operand expected in '-' operation.");
	}

	@Override
	public Void visit(Expression.Addition expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		expression.setType(type(leftType, rightType));
	}

	@Override
	public Void visit(Expression.Subtract expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isNumeric(leftType) && isNumeric(rightType))
			expression.setType(type(leftType, rightType));
		else Error.log(expression.pos, "Numeric operands expected in '-' operation.");
	}

	@Override
	public Void visit(Expression.Division expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isNumeric(leftType) && isNumeric(rightType))
			expression.setType(type(leftType, rightType));
		else Error.log(expression.pos, "Numeric operands expected in '/' operation.");
	}

	@Override
	public Void visit(Expression.Multiply expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isNumeric(leftType) && isNumeric(rightType))
			expression.setType(type(leftType, rightType));
		else Error.log(expression.pos, "Numeric operands expected in '*' operation.");
	}

	@Override
	public Void visit(Expression.Modulus expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isNumeric(leftType) && isNumeric(rightType))
			expression.setType(type(leftType, rightType));
		else Error.log(expression.pos, "Numeric operands expected in '%' operation.");
	}

	@Override
	public Void visit(Expression.ShiftLeft expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isInteger(leftType) && isInteger(rightType))
			expression.setType(Types.INT);
		else Error.log(expression.pos, "Integer operands expected in '<<' operation.");
	}

	@Override
	public Void visit(Expression.ShiftRight expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isInteger(leftType) && isInteger(rightType))
			expression.setType(Types.INT);
		else Error.log(expression.pos, "Integer operands expected in '>>' operation.");
	}

	@Override
	public Void visit(Expression.BitwiseAND expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isInteger(leftType) && isInteger(rightType))
			expression.setType(Types.INT);
		else Error.log(expression.pos, "Integer operands expected in '&' operation.");
	}

	@Override
	public Void visit(Expression.BitwiseOR expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isInteger(leftType) && isInteger(rightType))
			expression.setType(Types.INT);
		else Error.log(expression.pos, "Integer operands expected in '|' operation.");
	}

	@Override
	public Void visit(Expression.BitwiseXOR expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isInteger(leftType) && isInteger(rightType))
			expression.setType(Types.INT);
		else Error.log(expression.pos, "Integer operands expected in '^' operation.");
	}

	@Override
	public Void visit(Expression.BitwiseNOT expression) {
		analyse(expression.expression);
		Type type = expression.expression.type();
		if (isInteger(type)) expression.setType(Types.INT);
		else Error.log(expression.pos, "Integer operand expected in '~' operation.");
	}

	@Override
	public Void visit(Expression.Equal expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isComparable(leftType) && isComparable(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Operands type should be either numeric or string in '=' operation.");
	}

	@Override
	public Void visit(Expression.NotEqual expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isComparable(leftType) && isComparable(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Operands type should be either numeric or string in '!=' operation.");
	}

	@Override
	public Void visit(Expression.LessThan expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isComparable(leftType) && isComparable(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Operands type should be either numeric or string in '<' operations.");
	}

	@Override
	public Void visit(Expression.LessEqual expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isComparable(leftType) && isComparable(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Operands type should be either numeric or string in '<=' operation.");
	}

	@Override
	public Void visit(Expression.GreaterThan expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isComparable(leftType) && isComparable(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Operands type should be either numeric or string in '>' operation.");
	}
	
	@Override
	public Void visit(Expression.GreaterEqual expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isComparable(leftType) && isComparable(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Operands type should be either numeric or string in '>=' operation.");
	}

	@Override
	public Void visit(Expression.LogicalAND expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isBoolean(leftType) && isBoolean(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Required boolean type operands in logical or operation");
	}

	@Override
	public Void visit(Expression.LogicalOR expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (isBoolean(leftType) && isBoolean(rightType))
			expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Required boolean type operands in logical or operation");
	}

	@Override
	public Void visit(Expression.LogicalNOT expression) {
		analyse(expression.expression);
		Type type = expression.expression.type();
		if (isBoolean(type)) expression.setType(Types.BOOL);
		else Error.log(expression.pos, "Boolean operand expected in '!' operation.");
	}

	@Override
	public Void visit(Expression.Subscript expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (!(leftType instanceof ArrayType))
			Error.log(expression.pos, "");
		if (!(rightType instanceof IntType))
			Error.log(expression.pos, "");
		ArrayType arrType = (ArrayType) leftType;
		expression.setType(arrType.getElemType());
	}
	
	@Override
	public Void visit(Expression.NewArray expression) {
		analyse(expression.length);
		analyse(expression.initVal);
		Type lengthType = expression.length.type();
		Type initValType = expression.initVal.type();
		if (!(lengthType instanceof IntType))
			Error.log(expression.pos, "Array length must be an integer value.");
		expression.setType(initValType);
	}

	@Override
	public Void visit(Expression.Dot expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		expression.setType(expression.right.type);
	}

	@Override
	public Void visit(Expression.Call expression) {
		analyse(expression.caller);
		analyseAll(expression.arguments);
		if (expression.caller instanceof Identifier || expression.caller instanceof Dot) {
			
		}
	}
	
	@Override
	public Void visit(Expression.Conditional expression) {
		analyse(expression.first);
		analyse(expression.second);
		analyse(expression.third);
		Type firstType = expression.first.type();
		Type secondType = expression.second.type();
		Type thirdType = expression.third.type();
		if (firstType != Types.BOOL)
			Error.log(expression.pos, "Boolean type expected in first conditional expression.");
		if (!secondType.equals(thirdType))
			Error.log(expression.pos, "Second and third expression should be of same type in conditional expression.");
		expression.setType(firstType);
		return null;
	}
	
	@Override
	public Void visit(Expression.AddAssign expression) {		
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (!leftType.equals(rightType))
			Error.log(expression.pos, "Data type differs in both side of '+=' operation.");
		expression.setType(leftType);
		return null;
	}

	@Override
	public Void visit(Expression.SubAssign expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (!leftType.equals(rightType))
			Error.log(expression.pos, "Data type differs in both side of '-=' operation.");
		expression.setType(leftType);
		return null;
	}

	@Override
	public Void visit(Expression.Assignment expression) {
		analyse(expression.left);
		analyse(expression.right);
		Type leftType = expression.left.type();
		Type rightType = expression.right.type();
		if (!leftType.equals(rightType))
			Error.log(expression.pos, "Data type differs in both side of '=' operation.");
		expression.setType(leftType);
		return null;
	}
}
