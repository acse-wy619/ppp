#include <iostream>
#include <exception>
#include <string>
#include <stdexcept>
#include <cmath>
#include <vector>
#include <fstream>

/*
	Simple calculator

	Revision history
		Revised by bjarne stroustrup - Noveber 2013

	Modification History
		Modified by Goran Veletic - December 2017

	This program implements a basic expression calculator.
	Input from std::cin; output std::cout;
	The grammer for input is:

	Statement:
		Expression
		Print
		Quit
	Print
		;
	Quit
		q
	
	Calculator:
		Prompt
		Expression
		Output
	Expressin:
		Term
		Expression + Term
		Expression - Term
	Term:
		Primary
		Term * Primary
		Term / Primary
		Term % Primary
	Primary:
		Number
		( Expression )
		- Primary
		+ Primary
	Number
		floating-point-literal

	Input comes from std::cin through the Token_stream caled ts.
*/

// CONSTANTS
const char number = '8';	// number
const char quit	= 'q';	// token for quitting
const char print	= '\n';	// token for printing
const char result	= '=';	// used to indicate that what follows is a result
const char name	= 'a';	// name token
const char root	= 's';	// square root
const char power	= 'p';	// power
const char con		= 'c';	// constant
const char help	= 'h';	// help
const char from	= 'f';	// from
const char to		= 't';	// to
const char let		= 'L';	// declaration token

const std::string declkey	= "let";	// variable declaration
const std::string constkey	= "const";
const std::string sqrtkey	= "sqrt";	// sqrt key
const std::string powerkey	= "pow";
const std::string quitKey	= "quit";
const std::string helpkey	= "help";
const std::string fromKey	= "from";
const std::string toKey		= "to";
const std::string prompt	= "# ";


class Token {
	public:
		char kind;
		double value;
		std::string	name;

		Token (char ch) : kind (ch), name (""), value (0) {
		}

		Token (char ch, double val) : kind (ch), name(""), value (val) {
		}

		Token (char ch, std::string n) : kind (ch), name (n), value (0) {
		}

		void print();

};

class Token_stream {
	public:
		Token_stream () : full (false), buffer(0), fromFile (false) {
		}

		Token get();
		void putback (Token t);	// put a Token back
		void ignore (char c);
	private:
		bool fromFile;
		std::ifstream ifs;
		bool full;
		Token buffer;	// here is where we keep a Token put back using putback();
};

void Token_stream::putback (Token t) {
	if (full)
		throw std::runtime_error("Bad token. Token stream full");	

	buffer = t;		// copy to to buffer
	full = true;	// buffer is now full
}

Token Token_stream::get() {
	if (full) {	// do we already have a Token ready?
		full = false;	// remove Token from buffer
		return buffer;
	}

	if (ifs.eof() || ifs.fail())
		return Token(quit);

	char ch;
	if (fromFile)  {
		ifs.get(ch);
		ifs.close ();
	} else
		std::cin.get(ch);

	if (ch == 'H')
		ch |= 0x20;

	switch (ch) {
		case print:	// for "print"
		case quit:	// for "quit"
		case help:	// for help
		case '(':
		case ')':
		case '{':
		case '}':
		case '+':
		case '-':
		case '%':
		case '*':
		case '/': 
		case '=':
		case '!':	// factorial
		case ',':	// this is for cases like pow (a,b)
		case '^':	// for power 2 ^ 3 = 8
			return Token (ch);		// let each character represent itsefl
		case '.':	// floating point litteral
		case '0':	case '1':	case '2':	case '3':	case '4':
		case '5':	case '6':	case '7':	case '8':	case '9': {
			double val;

			if (fromFile)  {
				ifs.putback(ch);
				ifs >> val;
			} else {
				std::cin.putback(ch);	// put digit back into the input stream
				std::cin >> val;		// read a floating point number
			}

			return Token (number, val);
		}
		default:
			if (isspace(ch))
				return get();
			if (isalpha(ch) || ch == '_') {
				std::string s;
				s += ch;

				if (fromFile) {
					while (ifs.get(ch) && (isalpha(ch) || isdigit(ch)) || ch == '_')
						s += ch;

					ifs.putback (ch);
				} else {
					while (std::cin.get(ch) && (isalpha(ch) || isdigit(ch)) || ch == '_')
						s += ch;
				
					std::cin.putback (ch);
				}

				if (s == declkey)
					return Token (let);
				if (s == sqrtkey)
					return Token (root);
				if (s == powerkey)
					return Token (power);
				if (s == helpkey)
					return Token (help);
				if (s == fromKey) {
					std::cin >> s;
					ifs.open (s.c_str());
					fromFile = true;
					return Token (from, s);
				}
				if (s == toKey) {
					std::cin >> s;
					return Token (to, s);
				}
				if (s == quitKey) {
					ifs.close ();
					return Token (quit);
				}
			
				return Token (name, s);
			}

			throw std::runtime_error("Bad token.");	 
	}
}

void Token_stream::ignore (char c) { // c represents the kind of token
	if (full && c == buffer.kind) {
		full = false;
		return;
	}
	full = false;
	char ch = 0;
	while (std::cin.get(ch))
		if (ch == c)
			return;
}

class Variable {
	public:
		std::string name;
		double value;
		bool constant;

	Variable (std::string n, double v, bool c) : name (n), value (v), constant (c) {
	}
};

class Symbol_table {
		std::vector<Variable> var_table;
	public:
		double	get	(std::string s);
		void		set	(std::string s, double d);
		bool		declared	(std::string s);
		double	declare	(std::string var, double val, bool constant);
};	

double Symbol_table::get (std::string s) { // return the value of the Variable named s
	for (int i = 0;	i < var_table.size(); ++i)
		if (var_table[i].name == s)
			return var_table[i].value;

	throw std::runtime_error ("get: underfiended variable " + s);
}

void Symbol_table::set (std::string s, double d) { // set the variable named s to d
	for (int i = 0;  i < var_table.size(); ++i)
		if (var_table[i].name == s) {
			if (var_table[i].constant == true) {
				throw std::runtime_error (s + " is constant variable and cannot be changed");
				return;
			}
			var_table[i].value = d;
			return;
		}

	throw std::runtime_error ("set: underfined variable " + s);
}

bool Symbol_table::declared (std::string var) {
	for (int i = 0;  i < var_table.size(); ++i)
		if (var_table[i].name == var) 
			return true;

	return false;
}

double Symbol_table::declare (std::string var, double val, bool constant) {
	if (declared (var))
		throw std::runtime_error ("Declared twice.");

	var_table.push_back(Variable(var,val,constant));

	return val;
}

// Token_stream ts;
Symbol_table st;

void	calculator ();
double	statement (Token_stream& ts);
double	declaration (Token_stream& ts, bool constant);
double	expression (Token_stream& ts);	// read and evaluate expression
double	term (Token_stream& ts);
double	factorial (Token_stream& ts);
double	primary (Token_stream& ts);
void	clean_up_mess (Token_stream& ts);
void	print_help ();


int main (void) {
	try {
		std::cout << "Welcome to our simple calculator. ";
		std::cout << "Please enter expression using floating point numbers.\n";
		std::cout << "Available operations are +, -, * and /\n";
		std::cout << "To print type in '=' and to exit press 'x'\n";

		calculator();
		return 0;
	} catch (std::exception &e) {
		std::cerr << e.what() << "\n";
		return 1;
	} catch (...) {
		std::cerr << " unknown exception.\n";
		return 2;
	}

	return 0;
}

void calculator () {
	st.declare ("pi",	3.1415926535, true);
	st.declare ("e",	2.7182818284, true);
	bool fromFile = false;	
	bool toFile = false;
	std::ofstream ofs;
	Token_stream ts;
	while (1)
		try {
			if (!fromFile) 
				std::cout << prompt;
			Token t = ts.get();
			while (t.kind == print)
				t = ts.get();
			if (t.kind == quit) {
				if (toFile)
					ofs.close();
				return;
			}
			
			if (t.kind == from) {
				std::cout << "Reading from file " << t.name <<"\n";
				t = ts.get();
				fromFile = true;
			} 

			if (t.kind == to) {
				std::cout << "Writting to file " << t.name <<"\n";
				ofs.open (t.name.c_str());
				if (ofs.fail())
					std::cout << "Something went wrong.";
	
				t = ts.get();
				toFile = true;
				continue;
			}

			if (t.kind == help) {
				print_help();	
				clean_up_mess(ts);
				continue;
			}
			
			ts.putback (t);
			if (!toFile)
				std::cout << result << " " << statement(ts) << "\n";
			else
				ofs << result << " " << statement(ts) << "\n";

		} catch (std::runtime_error& e) {
			std::cerr << e.what() << "\n";
			clean_up_mess(ts);
		} 

	return;
}

double statement (Token_stream& ts) {
	Token t = ts.get();
	switch (t.kind) {
		case let :	{
			double d = declaration(ts, false);
			return d;
		}
		case con : {
			double d = declaration(ts, true);
			return d;
		}
		default	: 
			ts.putback(t);
			return expression(ts);
	}
}

double declaration (Token_stream& ts, bool constant) {
	Token t = ts.get();

	if (t.kind != name) 
		throw	std::runtime_error ("name expected in declaration");

	std::string var_name = t.name;

	Token t2 = ts.get();
	if (t2.kind != '=')		
		throw std::runtime_error (" = missing in delcaration of " + var_name);

	double d = expression (ts);
	st.declare (var_name, d, constant);
	return d;
}

double expression (Token_stream& ts) {
	double left = term (ts);	// read and evaluate an Exrpession
	while (true) {
		Token t = ts.get();		// get the next token

		switch (t.kind) {		// see which kind of token is in
			case '+':
				left	+= term(ts);	// read and evaluate a Term then do an add
				break;
			case '-':
				left	-= term(ts);	// read and evaluate a Term then do a subtract
				break;
			default :
				ts.putback(t);
				return left;	// return the value of Expression
		}
	}
}

double term (Token_stream& ts) {
	double left = factorial(ts);
	Token t = ts.get();
	while (true) {
		switch (t.kind) {
			case '%': {
				double d = primary(ts); 
				if (d == 0)
					throw std::runtime_error ("Cannot divide by zero");
			
		//		left = fmod (left, d);		// book option 1
				left = left - d * ((int) (left/d));		// my solution
/* 
	vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				int i1 = narrow_cast <int> (left); // << implemented in .h file provided
				int i2 = narrow_cast <int> (primary());
				if (d == 0)
					throw std::runtime_error ("Cannot divide by zero");

				left = i1 % i2;
	^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/
				t = ts.get();
				break;
			} 
			case '*':
				left *= factorial(ts);
				t = ts.get();
				break;
			case '/': {
				double divisor = factorial(ts);
				if (divisor == 0)
					throw std::runtime_error ("Cannot divide by zero");

				left /= divisor; 
				t = ts.get();
				break;
			}
			case '^': {
				double right = factorial(ts);
				double result = 0;

				for (result = 1; right; --right, result *= left);

				left = result;
				t = ts.get();
				break;
			}
			default :
				ts.putback(t);
				return left;
		}
	}  
}

double factorial (Token_stream& ts) {
	double left = primary(ts);
	Token t = ts.get();

	if (t.kind == '!') {
		int factorial = 1;
		for (int i = (int) left; i > 0; --i)
			factorial *= i;

		return factorial;
	}
	
	ts.putback(t);

	return left;
}

double primary (Token_stream& ts) {
	Token t = ts.get();
	double d = 0;
	switch (t.kind) {
		case '(':
			d = expression(ts);
			t = ts.get();
			
			if (t.kind != ')')
				throw std::runtime_error ("')' expected");

			return d;
		case '{':  // handle '{' expression '}'
			d = expression(ts);
			t = ts.get();

			if (t.kind != '}')
				throw std::runtime_error ("'}' expected");

			return d;
		case '-':
			d = primary(ts); 
			return d ? -d : 0;	// this is to avoid -0 error
		case number :	// we use '8' to represent a number
			return t.value;	//	return to number's valu
		case name : {
			Token t2 = ts.get();
			if (t2.kind == '=') {
				// check if variable is declared
				if (!st.declared (t.name)) 
					throw std::runtime_error ("variable is not declared. Use 'let " + t.name + " = '<value>' " +
														" to declare new variable.");

				d = expression (ts);
				st.set (t.name, d);
				return d;
			}
			ts.putback (t2);
			return st.get (t.name);
		}
		case root :
			t = ts.get ();
			if (t.kind != '(')
				throw std::runtime_error ("'(' expected");

			d = expression (ts);

			if (d < 0)
				throw std::runtime_error ("Cannot take square root of negative number");

			t = ts.get ();
	
			if (t.kind != ')')
				throw std::runtime_error ("')' expected");

			return sqrt (d);
		case power : {
			t = ts.get ();
			if (t.kind != '(')
				throw std::runtime_error ("'(' expected");

			d = expression (ts);
			
			t = ts.get ();
			if (t.kind != ',')
				throw std::runtime_error ("',' expected");

			int p = (int) expression (ts);
			t = ts.get ();
			if (t.kind != ')')
				throw std::runtime_error  ("')' expected");

			double result = 0;
			for (result = 1; p; --p, result *= d);
			return result;
		}
		default	:
			throw std::runtime_error ("primary expected");
	} 
	return t.value;
}

void clean_up_mess (Token_stream& ts) {
	ts.ignore(print);
}

void print_help () {
	std::cout << "Help is comming soon...\n";
}
