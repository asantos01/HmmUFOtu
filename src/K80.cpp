/*
 * K80.cpp
 *
 *  Created on: Mar 7, 2017
 *      Author: zhengqi
 */

#include "K80.h"
#include "ProgLog.h"

namespace EGriceLab {
using namespace std;
using namespace Eigen;

const string K80::name = "K80";
const Vector4d K80::pi = Vector4d::Constant(1.0 / 4);

istream& K80::read(istream& in) {
	string line, tag, value;
	while(in >> tag) {
		if(tag[0] == '#') { /* comment or header */
			std::getline(in, line); /* ignore the entire line */
			continue;
		}
		if(tag == "Type:") {
			in >> value; // read in model type
			if(value != modelType()) {
				errorLog << "Unmatched Model Type!" << endl;
				errorLog << "Trying to read in a " << value << " model into a " << modelType() << " object" << endl;
				in.setstate(ios_base::badbit);
				return in;
			}
		}
		else if(tag == "kappa:") {
			in >> kappa;
			std::getline(in, line); /* ignore the entire line */
			break;
		}
		else {
			errorLog << "Un-recognized line found in K80 Model input: tag: " << tag << endl << line << endl;
			in.setstate(ios_base::badbit);
			return in;
		}
	}

	return in;
}

ostream& K80::write(ostream& out) const {
	out << "# DNA Substitution Model" << endl;
	out << "Type: " << modelType() << endl;
	out << "kappa: " << kappa << endl;

	return out;
}

void K80::trainParams(const vector<Matrix4d>& Pv, const Vector4d& f) {
	/* estimate kappa */
	double Ti = 0, Tv = 0;
	for(vector<Matrix4d>::const_iterator P = Pv.begin(); P != Pv.end(); ++P) {
		Ti += (*P)(A, G) + (*P)(G, A) + (*P)(C, T) + (*P)(T, C);
		Tv += (*P)(A, C) + (*P)(A, T) + (*P)(C, A) + (*P)(C, G) + (*P)(G, C) + (*P)(G, T) + (*P)(T, A) + (*P)(T, G);
	}
	kappa = Ti / Tv;
}

} /* namespace EGriceLab */
