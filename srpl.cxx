/*  Copyright (c) 2005 Neil "Superna" ARMSTRONG
    This file is part of Superna's Reverse Polish Language.

    SRPL is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    SRPL is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with SRPL; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/** 	\file 	srpl.cxx
	\author Neil "Superna" ARMSTRONG
	\date 	05/03/2006
*/

#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <exception>

using namespace std;

map<string,long> vars;
vector<long> stack;
vector<long> tstack;
vector<char> cstack;
map<string,string> procs;

int execute(string expr);
void callproc(string name);

class Error : public exception
{
	private:
		string err;

	public:
		Error(const string & txt)
			: err(txt) {}

		operator string()
		{
			return err;
		}
		
		virtual ~Error() throw() {}
};

int ascii2num(char letter)
{
	if(letter >= '0' && letter <='9')
		return letter - '0';
	if(letter >= 'A' && letter <= 'F')
		return letter - 'A';
	if(letter >= 'f' && letter <= 'f')
		return letter - 'f';
	return 0;
}

long convertbase(int orig, string & str)
{
	unsigned pos = 0;
	long num = 0;
	while(pos < str.size())
	{
		int c = ascii2num(str[pos]);
		++pos;
		if(c < 0)
			continue;
		num = num*orig + c;	
	}
	return num;
}

int isnum(string & istr)
{
	char test = istr[istr.size()-1];
	switch(test)
	{
		case 'h':case 'H': return 16;
		case 'b':case 'B': return 2;
		case 'o':case 'O': return 8;
		case '0':case '1':case '2':case '3':case '4':
		case '5':case '6':case '7':case '8':case '9':
			return 10;
	}
	return -1;	
}

int getnum(string & istr)
{
	int base = isnum(istr);
	if(base)
		return convertbase(base,istr);
	else
		return -1;
}

int iscmd(string & istr)
{
	if(istr=="DROP")
		return 0;
	if(istr=="DUP")
		return 1;
	if(istr=="SWAP")
		return 2;
	if(istr=="PRINT")
		return 3;
	if(istr=="NLINE")
		return 4;
	if(istr=="PRINTLN")
		return 5;
	if(istr=="PSTACK")
		return 6;
	if(istr=="FLUSH")
		return 7;
	if(istr==">>")
		return 8;
	if(istr=="<<")
		return 9;
	if(istr=="DISP")
		return 10;
	return -1;
}

int iscalc(string & istr)
{
	switch(istr[0])
	{
		case '+': return 0;
		case '-': return 1;
		case '*': return 2;
		case '/': return 3;
	}
	return -1;	
}

int iscmp(string & istr)
{
	if( istr=="<" ) return 0;
	if( istr=="<=" ) return 1;
	if( istr==">" ) return 2;
	if( istr==">=" ) return 3;
	if( istr=="=" ) return 4;
	if( istr=="<>" ) return 5;
        return -1;
}

int issetvar(string & istr)
{
	if(istr[0] == '>')
		return 0;
	return -1;
}

int isstring(string & istr)
{
	if(istr[0]=='\'')
	{	
		if(istr[istr.size()-1]=='\'')
			return istr.size()-2;
		else
			throw Error("Needed second ' for string");
	}
	return -1;
}

void set_var(string & var, long val)
{
	vars[var] = val;
}

long get_var(string & var)
{
	if(vars.find(var)==vars.end())
		throw Error(string("Unknown Var '") + var + "'"); 
	return vars[var];
}

void doit(string & code)
{
	if(code[0] == '@')
	{	
		callproc(code.substr(1));
		return;
	}
	int id = isnum(code);
	if(id > 0)
	{	
		stack.push_back(getnum(code));
		return;
	}
	id = iscmd(code);
	if(id >= 0)
	{
		switch(id)
		{
			case 0:
				stack.pop_back();
				break;
			case 1:
				stack.push_back(stack.back());
				break;
			case 2:
			{
				long top1 = stack.back();
				stack[stack.size()-1] = stack[stack.size()-2];
				stack[stack.size()-2] = top1;
			} break;
			case 3:
				cout << stack.back();
				break;
			case 4:
				cout << endl;
				break;
			case 5:
				cout << stack.back() << endl;
				break;
			case 6:
			{
				cout << '{';
				for(size_t i = 0 ; i < stack.size() ; ++i)
					if(i != stack.size()-1)
						cout << stack[i] << ';';
					else
						cout << stack[i];
				cout << "}" << endl;
			} break;
			case 7:
				stack.clear();
				break;
			case 8:
				if(stack.size()<=0)
					throw Error("Empty Stack");
				tstack.push_back(stack.back());
				stack.pop_back();
				break;
			case 9:
				if(tstack.size()<=0)
					throw Error("Empty Temp Stack");
				stack.push_back(tstack.back());
				tstack.pop_back();
				break;
			case 10:
			{
				if((unsigned)stack.back() > cstack.size())
					throw Error("Not enough cars in car stack");
				string str;
				for(int i = 0 ; i < stack.back() ; ++i)
				{
					str = string(1,cstack.back()) + str;
					cstack.pop_back();
				}
				cout << str;
				stack.pop_back();
			} break;
		}
		return;
	}
	id = iscmp(code);
	if(id >= 0)
	{	
		if(stack.size() < 2)
			throw Error(string("Stack must be size=2 =") + code);
		long op2 = stack.back();
		stack.pop_back();
		long op1 = stack.back();
		stack.push_back(op2);
		switch(id)
		{
			case 0:
				if(op1 < op2) stack.push_back(1);
				else stack.push_back(0);
				break;
			case 1:
                                if(op1 <= op2) stack.push_back(1);
                                else stack.push_back(0);
				break;
			case 2:
                                if(op1 > op2) stack.push_back(1);
                                else stack.push_back(0);
				break;
			case 3:
                                if(op1 >= op2) stack.push_back(1);
                                else stack.push_back(0);
				break;
			case 4:
                                if(op1 == op2) stack.push_back(1);
                                else stack.push_back(0);
				break;
			case 5:
                                if(op1 != op2) stack.push_back(1);
                                else stack.push_back(0);
				break;
		}
		return;
	}
	id = iscalc(code);
        if(id>=0)
        {
		if(stack.size() < 2)
			throw Error(string("Stack must be size=2 =") + code);
		long op2 = stack.back();
                stack.pop_back();
                long op1 = stack.back();
                stack.pop_back();
                switch(id)
                {
                        case 0: stack.push_back(op1+op2); break;
			case 1: stack.push_back(op1-op2); break;
			case 2: stack.push_back(op1*op2); break;
			case 3: if(op2==0)
					throw Error(string("Division by 0 =") + code);
				stack.push_back(op1/op2); break;
			
		}
		return;
        }
	id=isstring(code);
	if(id>=0)
	{
		for(int i = 1 ; i <= id ; ++i)
			if(code[i]=='#')
				cstack.push_back(' ');
			else
				cstack.push_back(code[i]);
		stack.push_back(id);
		return;
	}
	id=issetvar(code);
	if(id>=0)
	{
		string ncode = code.substr(1);
		set_var(ncode, stack.back());
		stack.pop_back();
		return;
	}
	//put var "code" on top of stack
	stack.push_back(get_var(code));
}

void doit(vector<string> & code)
{
	for(size_t i = 0 ; i < code.size() ; ++i)
		doit(code[i]);
}

// if: if <condition> then <corps then> [else <corps else>] endif
void doif(istringstream & code)
{
	vector<string> condition, corpsthen, corpselse;
	string instr;
	
	//Corps CONDITION
	code >> instr;
	while(instr != "then" && !code.eof())
	{
		condition.push_back(instr);
		code >> instr;
	}
	if(code.eof())
		throw Error("Waiting then");
	//instr must be "then"
	//Corps THEN
	code >> instr;
	while(instr != "else" && instr != "endif" && !code.eof())
	{
		corpsthen.push_back(instr);
		code >> instr;
	}
	if(code.eof())
		throw Error("Waiting else or endif");
	//instr must be "else" or "endif"
	// Corps ELSE
	if(instr != "endif")
	{	
		code >> instr;
		while(instr != "endif" && !code.eof())
		{
			corpselse.push_back(instr);
			code >> instr;
		}
	}
	if(instr != "endif")
		throw Error("Waiting endif");
	//Execution
	doit(condition);
	if(stack.size()>0 && stack.back() >= 1)
		doit(corpsthen);
	else
		doit(corpselse);
}

void dofor(istringstream & code)
{
	// Recup le nom, val de depart, val de fin
	string varname;
	code >> varname;
	string sstart, send;
	long start, end;
	code >> sstart >> send;
	// If - get the last stack
	if(sstart == "-")
	{
		start = stack.back();
		stack.pop_back();
	}
	else
		start = atoi(sstart.c_str());
	
	// if - get the last stack
	if(send == "-")
	{
		end = stack.back();
		stack.pop_back();
	}
	else
		end = atoi(send.c_str());
	
	// Recupere le corps de boucle
	vector<string> corps;
	string istr;
	code >> istr;
	while(istr != "next" && !code.eof())
	{
		corps.push_back(istr);
		code >> istr;
	}
	if(istr != "next")
		throw Error("Waiting next");
	// Execute le corps pour chaque valeur
	for(int i = start ; i <= end ; ++i)
	{
		set_var(varname, i);
		doit(corps);
	}
}

void callproc(string name)
{
	if(procs.find(name) == procs.end())
		throw Error(string("Unknown Procedure ") + name);
	execute(procs[name]);
}

void addproc(istringstream & code, string name)
{
	string istr, corps;
	code >> istr;
	while(istr != "end" && !code.eof())
	{
		corps += " " + istr;
		code >> istr;
	}
	if(istr != "end")
		throw Error("Waiting end");
	procs[name] = corps;
}

void dowhile(istringstream & code)
{
	vector<string> condition, corps;
	string istr;
	code >> istr;
	while(istr != "do" && !code.eof())
	{
		condition.push_back(istr);
		code >> istr;
	}
	if(code.eof())
		throw Error("Waiting do");
	code >> istr;
	while(istr != "do" && !code.eof())
	{
		corps.push_back(istr);
		code >> istr;
	}
	if(istr != "done")
		throw Error("Waiting done");

	//Execution
	doit(condition);
	while(stack.size()>0 && stack.back() > 0)
	{
		doit(corps);
		doit(condition);
	}
}

int execute(string expr)
{
	istringstream code(expr);
	string instr;
	
	while(!code.eof())
	{
		code >> instr;
		if(instr[0] == ':')
			addproc(code, instr.substr(1));
		else if(instr == "if")
			doif(code);
		else if(instr == "for")
			dofor(code);
		else if(instr == "while")
			dowhile(code);
		else
			doit(instr);
	}
	return stack.back();
}

int main(int argc, char **argv)
{
	if(argc == 2 && strcmp(argv[1], "-h")==0)
	{
		cout << "Superna's Reverse Polish Language" << endl
		     << "Usage : srpl [-h] [number's]" << endl
		     << endl << "   -h   Show this help" << endl
		     << endl << "If you want more : <superna@na-prod.com>"
		     << endl;
		return 1;
	}
	
	// Put parameters on the stack
	int param;
	for(int i = 1 ; i < argc ; ++i)
	{
		int len = strlen(argv[i]);
		//cout << argv[i] << endl;
		if(argv[i][0] < '0' || argv[i][0] > '9' )
		{
			for(int j = 0 ; j < len ; ++j)
				cstack.push_back(argv[i][j]);
			stack.push_back(len);
			continue;
		}
		param = atoi(argv[i]);
		stack.push_back(param);
	}
	// Reading
	string line, tmp;
	getline(cin,tmp);
	while(!cin.eof() && tmp != ".")
	{
		if(tmp[0] != '#')
			line += " " + tmp;
		getline(cin,tmp);	
	}
	// Execution	
	try
	{
		execute(line);
	}
	catch(Error & exp)
	{
		cout << "unrecoverable Error : " << (string)exp << endl;
	}
}

