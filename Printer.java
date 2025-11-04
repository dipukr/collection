package saral;

import java.util.List;

public class Printer implements Statement.Visitor<String>, Expression.Visitor<String> {
	
	private StringBuffer source = new StringBuffer(10000);
	private List<Statement> statements;
	private int tabCount = 0;

	public Printer(List<Statement> statements) {
		this.statements = statements;
	}

	public void print() {
		printAlls(statements);
		System.out.println(source);
	}

	public void printAlls(List<Statement> statements) {
		for (Statement statement: statements) {
			align();
			print(statement);
			println();
		}
	}

	public void printAll(List<Expression> expressions) {
		var iter = expressions.iterator();
		while (iter.hasNext()) {
			print(iter.next());
			if (iter.hasNext())
				print(", ");
		}
	}

	public void printNames(List<String> names) {
		var iter = names.iterator();
		while (iter.hasNext()) {
			print(iter.next());
			if (iter.hasNext())
				print(", ");
		}
	}
	
	public String print(Statement statement) {
		return statement.accept(this);
	}

	public String print(Expression expression) {
		return expression.accept(this);
	}
	
	public void print(char v) {source.append(v);}
	public void print(int v) {source.append(v);}
	public void print(long v) {source.append(v);}
	public void print(double v) {source.append(v);}
	public void print(String s) {source.append(s);}
	
	public void println() {
		source.append('\n');
	}
	
	public void indent() {tabCount += 1;}
	public void dedent() {tabCount -= 1;}

	public void align() {
		for (int a = 0; a < tabCount; a++)
			source.append('\t');
	}
	
	@Override
	public String visit(Statement.ClassDecl statement) {
		return null;
	}

	@Override
	public String visit(Statement.Function statement) {
		print("function ");
		print(statement.sym.name);
		print('(');
		var iter = statement.params.iterator();
		while (iter.hasNext()) {
			Symbol param = iter.next();
			print(param.type.getName());
			print(' ');
			print(param.name);
			if (iter.hasNext())
				print(", ");
		}
		for (Symbol param: statement.params) {
			print(param.type.getName());
			print(" ");
			
		}	
		print(") {\n");
		indent();
		print(statement.statement);
		print('}');
	}
	
	@Override
	public String visit(Statement.Block statement) {
		print('{');
		println();
		indent();
		printAlls(statement.statements);
		dedent();
		align();
		print("} ");
	}
	
	@Override
	public String visit(Statement.VarDecl statement) {
		print("var ");
		print(statement.sym.name);
		print(" = ");
		print(statement.initVal);
	}

	@Override
	public String visit(Statement.If statement) {
		print("if (");
		print(statement.condition);
		print(")");
		print(statement.statement1);
		if (statement.statement2 != null) {
			println();
			align();
			print("else ");
			print(statement.statement2);
		}
	}

	@Override
	public String visit(Statement.While statement) {
		print("while (");
		print(statement.condition);
		print(")");
		print(statement.statement);
	}

	@Override
	public String visit(Statement.Foreach statement) {
		
	}

	@Override
	public String visit(Statement.Break statement) {
		print("break");
	}

	@Override
	public String visit(Statement.Return statement) {
		print("return ");
		print(statement.expression);
	}

	@Override
	public String visit(Statement.Assert statement) {
		print("assert ");
		print(statement.condition);
	}

	@Override
	public String visit(Statement.Console statement) {
		print("console(");
		printAll(statement.expressions);
		print(")");
	}

	@Override
	public String visit(Statement.Expresion statement) {
		print(statement.expression);
	}
	
	@Override
	public String visit(Expression.Literal expression) {
		print('"');
		Value val = expression.value;
		String str = switch (val.type) {
			case Value.STR -> (String) val.val;
			case Value.INT -> String.valueOf((int) val.val);
			case Value.NUM -> String.valueOf((double) val.val);
			case Value.BOOL -> String.valueOf((boolean) val.val);
			default -> "null";
		};
		print(str);
		print('"');
	}
	
	@Override
	public String visit(Expression.ArrayLiteral expression) {
		print("[");
		printAll(expression.expressions);
		print("]");
	}
	
	@Override
	public String visit(Expression.Identifier expression) {
		print(expression.sym.name);
	}

	@Override
	public String visit(Expression.Negation expression) {
		print("-");
		print(expression.expression);
	}
	
	@Override
	public String visit(Expression.Addition expression) {
		return print(expression.left) + 
				" + " + print(expression.right);
	}

	@Override
	public String visit(Expression.Subtract expression) {
		return print(expression.left) + 
				" - " + print(expression.right);
	}

	@Override
	public String visit(Expression.Multiply expression) {
		return print(expression.left) + 
				" * " + print(expression.right);
	}

	@Override
	public String visit(Expression.Division expression) {
		return print(expression.left) + 
				" / " + print(expression.right);
	}

	@Override
	public String visit(Expression.Modulus expression) {
		return print(expression.left) + 
				" % " + print(expression.right);
	}

	@Override
	public String visit(Expression.ShiftLeft expression) {
		return print(expression.left) + 
				" << " + print(expression.right);
	}

	@Override
	public String visit(Expression.ShiftRight expression) {
		return print(expression.left) + 
				" >> " + print(expression.right);
	}

	@Override
	public String visit(Expression.BitwiseOR expression) {
		return print(expression.left) + 
				" | " + print(expression.right);
	}

	@Override
	public String visit(Expression.BitwiseXOR expression) {
		return print(expression.left) + 
				" ^ " + print(expression.right);
	}

	@Override
	public String visit(Expression.BitwiseAND expression) {
		return print(expression.left) + 
				" & " + print(expression.right);
	}

	@Override
	public String visit(Expression.BitwiseNOT expression) {
		return "~" + print(expression.expression);
	}

	@Override
	public String visit(Expression.Equal expression) {
		return print(expression.left) + 
				" == " + print(expression.right);
	}

	@Override
	public String visit(Expression.NotEqual expression) {
		return print(expression.left) + 
				" != " + print(expression.right);
	}

	@Override
	public String visit(Expression.LessThan expression) {
		return print(expression.left) + 
				" < " + print(expression.right);
	}

	@Override
	public String visit(Expression.LessEqual expression) {
		return print(expression.left) + 
				" <= " + print(expression.right);
	}

	@Override
	public String visit(Expression.GreaterThan expression) {
		return print(expression.left) + 
				" > " + print(expression.right);
	}

	@Override
	public String visit(Expression.GreaterEqual expression) {
		return print(expression.left) + 
				" >= " + print(expression.right);
	}

	@Override
	public String visit(Expression.LogicalOR expression) {
		return print(expression.left) + 
				" or " + print(expression.right);
	}

	@Override
	public String visit(Expression.LogicalAND expression) {
		return print(expression.left) + 
				" and " + print(expression.right);
	}

	@Override
	public String visit(Expression.LogicalNOT expression) {
		return "!" + print(expression.expression);
	}
	
	@Override
	public String visit(Expression.Subscript expression) {
		print(expression.left);
		print("[");
		print(expression.right);
		print("]");
	}
	
	@Override
	public String visit(Expression.NewArray expression) {
		print('[');
		print(expression.length);
		print(": ");
		print(expression.initVal);
		print(']');
	}
	
	@Override
	public String visit(Expression.Dot expression) {
		
	}

	@Override
	public String visit(Expression.Call expression) {
		print(expression.caller);
		print("(");
		printAll(expression.arguments);
		print(")");
	}
	
	@Override
	public String visit(Expression.Conditional expression) {
		return print(expression.first) + " ?" +
			print(expression.second) + " : " +
			print(expression.third);
	}
	
	@Override
	public String visit(Expression.AddAssign expression) {
		return print(expression.left) + 
				" += " + print(expression.right);
	}

	@Override
	public String visit(Expression.SubAssign expression) {
		return print(expression.left) + 
				" -= " + print(expression.right);
	}

	@Override
	public String visit(Expression.Assignment expression) {
		return print(expression.left) + 
				" = " + print(expression.right);
	}
}
