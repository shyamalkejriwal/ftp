//get a copy of string s
char *get_str(const char *s){
	char *s1 = new char[strlen(s)+1];
	strcpy(s1,s);
	return s1;
}

//parse string(ftp terminal command) to get arguments
//Rules
//1. Arguments are delimited by white space(s), which is/are either (a) space(s) or (a) tab(s).
//2. A string surrounded by double quotation marks ("string") is interpreted as a single argument, 
//regardless of white space contained within. A quoted string can be embedded in an argument.
//3. A double quotation mark preceded by a backslash (\") is interpreted as a literal double quotation mark character (").
//4. Any character 'ch' preceded by a backslash (\ch) is interpreted as a literal character 'ch' (including '\').

int pargs(const char *s, char ***u){	//s to be parsed, *u points to array of arguments
	vector<string> v;
	int count = 0;
	v.push_back(string(""));
	while(*s){
		if(*s == '\"'){ //quote seen
			while(*(++s) != '\"'){ //look for matching quote
				if(*s == '\\') s++; //Rule 3 and Rule 4
				if(*s == '\0'){	//matching quote not found
					cout << "Invalid argument(s): Missing \"\n";
					return -1;
				}
				v[count] += *s;
			}
			s++;
		}
		else if(*s == '\\'){ //Rule 4
			if(*(++s) == '\0'){ //stray backslash
				cout << "Invalid arguments(s): Stray \\\n";
				return -1;
			}
			v[count] += *s;
			s++;
		}
		else if((*s == ' ')||(*s == '\t')){ //Rule 1
			while((*s == ' ')||(*s == '\t')) s++; //ignore all spaces and tabs
			if(*s){
				count++;	//next argument
				v.push_back(string(""));
			}
		}
		else{
			v[count] += *s;
			s++;
		}
	}

	count += 1;
	char **t;
	t = new char *[count];
	for(int i = 0; i < count; i++){
		t[i] = get_str(v[i].c_str());
	}
	*u = t;
	return count;
}