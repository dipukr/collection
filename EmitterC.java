package saral;

import java.util.HashMap;
import java.util.List;
import java.util.Map;

import saral.Expression.ArrayLiteral;
import saral.Expression.NewArray;

import java.io.FileWriter;

public class EmitterC implements Statement.Visitor<Void>, Expression.Visitor<Void> {
	
	private StringBuffer source = new StringBuffer(10000);
	private Map<String, String> primitiveTbl;
	private List<Statement> statements;
	
	public EmitterC(List<Statement> statements) {
		this.statements = statements;
		this.primitiveTbl = new HashMap<>();
	}
	
	public void init() {
		primitiveTbl.put("bool", "bool");
		primitiveTbl.put("int", "int");
		primitiveTbl.put("num", "double");
	}
	
	public void premit() {
		String a = """
		#include <stdbool.h>
		#include <assert.h>
		#include <stdio.h>
		#include <stdint.h>
		#include <stdlib.h>
		struct str {char *data; int length;};
		struct array_bool {bool *data; int length;};
		struct array_int {int *data; int length;};
		struct array_num {double *data; int length;};
		struct array_str {struct str *data; int length;};\n
		""";
		source.append(a);		
	}

	public void emit() {
		premit();
		emitStatements(statements);
		try (var writer = new FileWriter("saral.c")) {
			writer.write(source.toString());
			writer.flush();
			writer.close();
		} catch (Exception e) {
			Error.fatal(e.getMessage());
		}
	}

	public void emitStatements(List<Statement> statements) {
		for (Statement statement: statements) {
			emit(statement);
		}
	}

	public void emitExpressions(List<Expression> expressions) {
		var iter = expressions.iterator();
		while (iter.hasNext()) {
			emit(iter.next());
			if (iter.hasNext())
				emit(", ");
		}
	}

	public void emitNames(List<String> names) {
		var iter = names.iterator();
		while (iter.hasNext()) {
			emit(iter.next());
			if (iter.hasNext())
				emit(", ");
		}
	}
	
	public void emitAlls(List<Statement> statements) {
		for (Statement statement: statements)
			emit(statement);
	}

	public void emitAll(List<Expression> expressions) {
		for (Expression expression: expressions)
			emit(expression);
	}
	
	public void emit(Statement statement) {
		statement.accept(this);
	}

	public void emit(Expression expression) {
		expression.accept(this);
	}
	
	public void emit(char v) {source.append(v);}
	public void emit(int v) {source.append(v);}
	public void emit(long v) {source.append(v);}
	public void emit(double v) {source.append(v);}
	public void emit(String s) {source.append(s);}
	
	public void emitType(Type type) {
		source.append(type.getName());
	}
	
	public void emitParams(List<Symbol> params) {
		emit('(');
		var iter = params.iterator();
		while (iter.hasNext()) {
			Symbol param = iter.next();
			emitType(param.type);
			emit(' ');
			emit(param.name);
			if (iter.hasNext())
				emit(',');
		}
		emit(')');
	}
	
	public void emitLiteral(Value val) {
		String str = switch (val.type) {
			case Value.STR -> String.format("\"%s\"", (String) val.val);
			case Value.INT -> String.valueOf((int) val.val);
			case Value.NUM -> String.valueOf((double) val.val);
			case Value.BOOL -> String.valueOf((boolean) val.val);
			default -> "NULL";
		};
		emit(str);
	}
	
	public void visit(Statement.ClassDecl statement) {
//		emit("class ");
//		emit(statement.name);
//		emit('{');
//		for (Statement field: statement.fields) {
//			if (field instanceof VarDecl varDecl) {
//				
//			}
//		}
//		for (Statement function: statement.functions) {
//			
//		}
//		emit('}');
	}

	public void visit(Statement.Function statement) {
		Symbol sym = statement.sym;
		Type type = sym.type;
		String name = sym.name;
		List<Symbol> params = statement.params;
		Statement body = statement.statement;
		emitType(type);
		emit(' ');
		emit(name);
		if (name.equals("main"))
			emit("(int argc, const char **argv)");
		else emitParams(params);
		emit(body);
	}
	
	public void visit(Statement.Block statement) {
		emit('{');
		emitStatements(statement.statements);
		emit("} ");
	}
	
	public String getArrayName(String name) {
		if (Primitive.isPrimitive(name))
			return String.format("array_%s", name);
		return String.format("array_object");
	}
	
	public String getCtypeName(String name) {
		if (name.equals("bool")) return name;
		if (name.equals("int")) return name;
		if (name.equals("num")) return "double";
		return String.format("struct %s", name);
	}
	
	public void visit(Statement.VarDecl statement) {
		Symbol sym = statement.sym;
		Type type = sym.type;
		String name = sym.name;
		boolean mutable = statement.mutable;
		Expression initVal = statement.initVal;
		
		if (type instanceof ArrayType arrayType) {
			String arrayName = getArrayName(arrayType.getElemType().getName());
			String typeName = arrayType.getElemType().getName();
			String ctypeName = getCtypeName(typeName);
			if (initVal instanceof NewArray newArr) {
				emit(String.format("struct %s *%s = (struct %s*) malloc(sizeof(struct %s));",
						arrayName, name, arrayName, arrayName));
				emit(String.format("%s->length=", name));
				emit(newArr.length);
				emit(';');
				emit(String.format("%s->data=(%s*)malloc(", name, typeName));
				emit(newArr.length);
				emit(String.format("*sizeof(%s));", typeName));
				emit(String.format("for(int __i=0;__i<%s->length;__i++)%s->data[__i]=", name, name));
				emit(newArr.initVal);
				emit(';');
			} else if (initVal instanceof ArrayLiteral arrlit) {
				List<Expression> literals = arrlit.expressions;
				emit(String.format("struct %s *%s = (struct %s*) malloc(sizeof(struct %s));",
						arrayName, name, arrayName, arrayName));
				emit(String.format("%s->length=%d;", name, literals.size()));
				emit(String.format("%s->data=(%s*)malloc(%d*sizeof(%s));",
						name, typeName, literals.size(), typeName));
				emit(String.format("%s __%s__[]={", ctypeName, name));
				emitExpressions(literals);
				emit("};");
				emit(String.format("for(int __i=0;__i<%s->length;__i++)%s->data[__i]=__%s__[__i];",
						name, name, name));
			}
		} else {
			if (!mutable) emit("const ");
			emit(type.getName());
			emit(' ');
			emit(name);
			if (initVal != null) {
				emit('=');
				emit(initVal);
				emit(';');
			}
		}
	}

	public void visit(Statement.If statement) {
		emit("if (");
		emit(statement.condition);
		emit(")");
		emit(statement.statement1);
		if (statement.statement2 != null) {
			emit("else ");
			emit(statement.statement2);
		}
	}

	public void visit(Statement.While statement) {
		emit("while (");
		emit(statement.condition);
		emit(")");
		emit(statement.statement);
	}

	public void visit(Statement.Foreach statement) {
		
	}

	public void visit(Statement.Break statement) {
		emit("break;");
	}

	public void visit(Statement.Return statement) {
		emit("return ");
		emit(statement.expression);
		emit(';');
	}

	public void visit(Statement.Assert statement) {
		emit("assert(");
		emit(statement.condition);
		emit(");");
	}

	public void visit(Statement.Console statement) {
		emit("printf(");
		emit('"');
		for (Expression expression: statement.expressions)
			emit("%d");
		emit("\\n");
		emit('"');
		emit(',');
		var iter = statement.expressions.iterator();
		while (iter.hasNext()) {
			emit(iter.next());
			if (iter.hasNext())
				emit(',');
		}
		emit(");");
	}

	public void visit(Statement.Expresion statement) {
		emit(statement.expression);
		emit(';');
	}
	
	public void visit(Expression.Literal expression) {
		emitLiteral(expression.value);
	}
	
	public void visit(Expression.ArrayLiteral expression) {
		emit("{");
		emitExpressions(expression.expressions);
		emit("}");
	}
	
	public void visit(Expression.Identifier expression) {
		emit(expression.sym.name);
	}

	public void visit(Expression.Negation expression) {
		emit("-");
		emit(expression.expression);
	}
	
	public void visit(Expression.Addition expression) {
		emit(expression.left);
		emit("+");
		emit(expression.right);
	}

	public void visit(Expression.Subtract expression) {
		emit(expression.left);
		emit("-");
		emit(expression.right);
	}

	public void visit(Expression.Multiply expression) {
		emit(expression.left);
		emit("*");
		emit(expression.right);
	}

	public void visit(Expression.Division expression) {
		emit(expression.left);
		emit("/");
		emit(expression.right);
	}

	public void visit(Expression.Modulus expression) {
		emit(expression.left);
		emit("%");
		emit(expression.right);
	}

	public void visit(Expression.ShiftLeft expression) {
		emit(expression.left);
		emit("<<");
		emit(expression.right);
	}

	public void visit(Expression.ShiftRight expression) {
		emit(expression.left);
		emit(">>");
		emit(expression.right);
	}

	public void visit(Expression.BitwiseOR expression) {
		emit(expression.left);
		emit("|");
		emit(expression.right);
	}

	public void visit(Expression.BitwiseXOR expression) {
		emit(expression.left);
		emit("^");
		emit(expression.right);
	}

	public void visit(Expression.BitwiseAND expression) {
		emit(expression.left);
		emit("&");
		emit(expression.right);
	}

	public void visit(Expression.BitwiseNOT expression) {
		emit("~");
		emit(expression.expression);
	}

	public void visit(Expression.Equal expression) {
		emit(expression.left);
		emit("==");
		emit(expression.right);
	}

	public void visit(Expression.NotEqual expression) {
		emit(expression.left);
		emit("!=");
		emit(expression.right);	
	}

	public void visit(Expression.LessThan expression) {
		emit(expression.left);
		emit("<");
		emit(expression.right);
	}

	public void visit(Expression.LessEqual expression) {
		emit(expression.left);
		emit("<=");
		emit(expression.right);	
	}

	public void visit(Expression.GreaterThan expression) {
		emit(expression.left);
		emit(">");
		emit(expression.right);	
	}

	public void visit(Expression.GreaterEqual expression) {
		emit(expression.left);
		emit(">=");
		emit(expression.right);
	}

	public void visit(Expression.LogicalOR expression) {
		emit(expression.left);
		emit("||");
		emit(expression.right);
	}

	public void visit(Expression.LogicalAND expression) {
		emit(expression.left);
		emit("&&");
		emit(expression.right);
	}

	public void visit(Expression.LogicalNOT expression) {
		emit("!");
		emit(expression.expression);
	}
	
	public void visit(Expression.Subscript expression) {
		emit(expression.left);
		emit("[");
		emit(expression.right);
		emit("]");
	}
	
	public void visit(Expression.NewArray expression) {
		
	}
	
	public void visit(Expression.Dot expression) {
		emit(expression.left);
		emit("->");
		emit(expression.right);
	}
	
	public void visit(Expression.Call expression) {
		emit(expression.caller);
		emit("(");
		emitExpressions(expression.arguments);
		emit(")");
	}
	
	public void visit(Expression.Conditional expression) {
		emit(expression.first);
		emit("?");
		emit(expression.second);
		emit(":");
		emit(expression.third);
	}
	
	public void visit(Expression.AddAssign expression) {
		emit(expression.left);
		emit("+=");
		emit(expression.right);
	}

	public void visit(Expression.SubAssign expression) {
		
	}

	public void visit(Expression.Assignment expression) {
		emit(expression.left);
		emit("=");
		emit(expression.right);
	}
}
