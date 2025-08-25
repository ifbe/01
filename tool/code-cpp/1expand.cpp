#include "util.hpp"
#define CONFIG_PRINT_HIDE_COMMENTED 0






std::string translatepinname(std::string s0, design* in, wiredef* wi)
{
	//1 pinout: name unchanged
	int j;
	for(j=0;j<in->_pinout.size();j++){
		if(s0 == in->_pinout[j]->name){
			return wi->pinname[j];
		}
	}

	//2 pinin: refname/pinname
	for(j=0;j<in->_pinin.size();j++){
		if(s0 == in->_pinin[j]->name){
			return wi->chipname + "/" + s0;
		}
	}

	//3 notfound: maybe vcc,gnd...
	return "?" + s0;
}
void expand_real(design* out, design* in, wiredef* wi){
	printf("in: name=%s\n", in->name.c_str());
	printf("wi: name=%s, pin=%s...\n", wi->chipname.c_str(), wi->pinname[0].c_str());

	//inner pin: rename and insert
	for(auto& p : in->_pinin){
		out->_pinin.push_back(new pindef(wi->chipname + "/" + p->name));
	}

	//inner chip: rename and insert
	for(auto& p : in->_chip){
		out->_chip.push_back(new chipdef(wi->chipname + "/" + p->name, p->cname));
	}

	//wiring
	for(auto& p : in->_connect){
		wiredef* tmp = new wiredef(wi->chipname + "/" + p->chipname);

		for(auto q : p->pinname){
			std::string s = translatepinname(q, in, wi);
			tmp->addpin(s);
		}

		out->_connect.push_back(tmp);
	}

	//layout
	for(auto& p : in->_layout){
		printf("xxxxxxxxxxxxxxxxx\n");
	}
}

void expand_onelayer(session* sess, design* ds, int last){
	int sz = ds->_chip.size();
	int j;
	design* founddesign;
	wiredef* foundwire;
	for(j=last;j<sz;j++){
		std::string nickname = ds->_chip[j]->name;
		std::string origname = ds->_chip[j]->cname;
		printf("%d: nickname=%s origname=%s{\n", j, nickname.c_str(), origname.c_str());

		founddesign = finddesign(sess, origname);
		if(founddesign == 0){
			printf("not found in file\n");
			goto prepnext;
		}
		if(founddesign->_chip.size() == 0){
			printf("this is already base element\n");
			goto prepnext;
		}

		foundwire = findwire(ds, nickname);
		if(foundwire == 0){
			printf("not found in logic\n");
			goto prepnext;
		}

		expand_real(ds, founddesign, foundwire);

		//comment this chip and this wire
		ds->_chip[j]->name = "//" + nickname;
		foundwire->chipname = "//" + nickname;

prepnext:
		printf("}\n");
	}
}

design* expand(session* sess, std::string name){
	int j;
	design* found = finddesign(sess, name);
	if(0 == found)return 0;
	printf("\n");

	design* theone = new design(found);
	int prev = 0;
	int curr = 0;
	for(int j=0;j<3;j++){
		curr = theone->_chip.size();
		expand_onelayer(sess, theone, prev);
		prev = curr;
	}
	printf("\n");

	printdesign(theone);

	theone->shrinkunused();

	//printdesign(theone);

	//delete theone;
	return theone;
}
