char *get_str(const char *s){
	char *s1 = new char[strlen(s)+1];
	strcpy(s1,s);
	return s1;
}


int pargs(const char *s, char ***u){
	vector<string> v;
	int count = 0;
	v.push_back(string(""));
	while(*s){
		if(*s == '\"'){
			while(*(++s) != '\"'){
				if(*s == '\\') s++;
				if(*s == '\0'){
					cout << "Invalid argument(s): Missing \"\n";
					return -1;
				}
				v[count] += *s;
			}
			s++;
		}
		else if(*s == '\\'){
			if(*(++s) == '\0'){
				cout << "Invalid arguments(s): Stray \\\n";
				return -1;
			}
			v[count] += *s;
			s++;
		}
		else if((*s == ' ')||(*s == '\t')){
			while((*s == ' ')||(*s == '\t')) s++;
			if(*s){
				count++;
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