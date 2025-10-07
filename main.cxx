#include <iostream>
#define logger(a) std::cout << a << std::endl

struct Address {
	Address() {
		logger("Address created");
	}
	~Address() {
		logger("Address destroyed");
	}
};

struct User {
	Address addr;
	User() {
		logger("User created");
	}
	~User() {
		logger("User destroyed");
	}
};

User getUser() {
	User u;
	return u;
}


void log(uint a) {
	logger(a);
}
struct Add;
struct Sub;
struct Visitor {
    void visit(Add &expr);
    void visit(Sub &expr);
};

struct Expr
{
    int pos;
    virtual void accept(Visitor &visitor)=0;
};

struct Add : Expr
{
    Expr &left;
    Expr &right;
    Add(Expr &left, Expr &right) : left(left), right(right){}
    Expr& left() const {return left;}
    void accept(Visitor &visitor) {visitor.visit(*this);}
};

struct Sub : Expr
{
    Expr &left;
    Expr &right;
    Sub(Expr &left, Expr &right) : left(left), right(right){}
    void accept(Visitor &visitor) {visitor.visit(*this);}
};
struct Printer : Visitor
{
    
};



















int main(int argc, const char **argv)
{
	log(-100);
	return EXIT_SUCCESS;
}