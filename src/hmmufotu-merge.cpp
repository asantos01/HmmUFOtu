/*
 * hmmufotu-merge.cpp
 *  merge two or more OTUTables into a single one
 *  Created on: Jul 18, 2018
 *      Author: zhengqi
 *      Version: v1.1
 */

#include <iostream>
#include <fstream>
#include <cctype>
#include <cfloat>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <boost/unordered_set.hpp>
#include <boost/lexical_cast.hpp>
#include "HmmUFOtu.h"
#include "HmmUFOtu_main.h"

using namespace std;
using namespace EGriceLab;
using namespace EGriceLab::HmmUFOtu;

/* default values */
static const string TABLE_FORMAT = "table";
typedef boost::unordered_set<PTUnrooted::PTUNodePtr> OTUSet;


/**
 * Print introduction of this program
 */
void printIntro(void) {
	cerr << "Merge two or more OTUTables generated by hmmufotu-sum or other utils" << endl;
}

/**
 * Print the usage information
 */
void printUsage(const string& progName) {
	cerr << "Usage:    " << progName << "  <OTU-FILE1> <OTU-FILE2> [OTU-FILE3 ...] [options]" << endl
		 << "OTU-FILE        FILE           : OTUFile from hmmufotu-sum or other utils" << endl
		 << "Options:    -o  FILE           : write merged OTU to FILE instead of stdout" << endl
		 << "            -t  FILE           : OTU tree output" << endl
		 << "            --db  STR          : database name (prefix) used to generate these OTUs, required only if -t is requested" << endl
		 << "            -v  FLAG           : enable verbose information, you may set multiple -v for more details" << endl
		 << "            --version          : show program version and exit" << endl
		 << "            -h|--help          : print this message and exit" << endl;
}

int main(int argc, char* argv[]) {
	string dbName, ptuFn;
	vector<string> inFiles;
	string otuFn, treeFn;
	ifstream ptuIn;
	ofstream otuOut, treeOut;

	/* parse options */
	CommandOptions cmdOpts(argc, argv);
	if(cmdOpts.empty() || cmdOpts.hasOpt("-h") || cmdOpts.hasOpt("--help")) {
		printIntro();
		printUsage(argv[0]);
		return EXIT_SUCCESS;
	}

	if(cmdOpts.hasOpt("--version")) {
		printVersion(argv[0]);
		return EXIT_SUCCESS;
	}

	if(!(cmdOpts.numMainOpts() >= 2)) {
		cerr << "Error:" << endl;
		printUsage(argv[0]);
		return EXIT_FAILURE;
	}

	inFiles = cmdOpts.getMainOpt();

	if(cmdOpts.hasOpt("-o"))
		otuFn = cmdOpts.getOpt("-o");

	if(cmdOpts.hasOpt("-t"))
		treeFn = cmdOpts.getOpt("-t");

	if(cmdOpts.hasOpt("--db"))
		dbName = cmdOpts.getOpt("--db");

	if(cmdOpts.hasOpt("-v"))
		INCREASE_LEVEL(cmdOpts.getOpt("-v").length());

	/* validate options */
	if(!treeFn.empty() && dbName.empty()) {
		cerr << "--db is required when -t is requested" << endl;
		return EXIT_FAILURE;
	}

	/* set filenames */
	ptuFn = dbName + PHYLOTREE_FILE_SUFFIX;

	/* open inputs */


	/* open outputs */
	if(!otuFn.empty()) {
		otuOut.open(otuFn.c_str());
		if(!otuOut.is_open()) {
			cerr << "Unable to write to '" << otuFn << "': " << ::strerror(errno) << endl;
			return EXIT_FAILURE;
		}
	}
	ostream& out = otuOut.is_open() ? otuOut : cout;

	if(!treeFn.empty()) {
		treeOut.open(treeFn.c_str());
		if(!treeOut.is_open()) {
			cerr << "Unable to write to '" << treeFn << "': " << ::strerror(errno) << endl;
			return EXIT_FAILURE;
		}
		/* only open PTU when tree is requested */
		ptuIn.open(ptuFn.c_str(), ios_base::in | ios_base::binary);
		if(!ptuIn.is_open()) {
			cerr << "Unable to open PTU data '" << ptuFn << "': " << ::strerror(errno) << endl;
			return EXIT_FAILURE;
		}
	}

	PTUnrooted ptu;
	/* loading database file when required */
	if(ptuIn.is_open()) {
		if(loadProgInfo(ptuIn).bad())
			return EXIT_FAILURE;
		ptu.load(ptuIn);
		if(ptuIn.bad()) {
			cerr << "Unable to load Phylogenetic tree data '" << ptuFn << "': " << ::strerror(errno) << endl;
			return EXIT_FAILURE;
		}
		infoLog << "Phylogenetic tree loaded" << endl;
	}

	/* merge OTUTables */
	infoLog << "Merging OTUTables" << endl;
	OTUTable otuMerged;
	for(vector<string>::const_iterator inFn = inFiles.begin(); inFn != inFiles.end(); ++inFn) {
		/* open input */
		ifstream otuIn(inFn->c_str());
		if(!otuIn.is_open()) {
			cerr << "Unable to open '" << *inFn << "': " << ::strerror(errno) << endl;
			return EXIT_FAILURE;
		}
		infoLog << *inFn << endl;
		if(readProgInfo(otuIn).bad())
			return EXIT_FAILURE;
		OTUTable otuTable;
		otuTable.load(otuIn, TABLE_FORMAT);

		otuMerged += otuTable;
	}

	/* write output */
	infoLog << "Writing merged OTUTable" << endl;
	writeProgInfo(out, string(" OTU table merged by ") + argv[0]);
	otuMerged.save(out, TABLE_FORMAT);

	/* write the tree */
	if(treeOut.is_open()) {
		infoLog << "Writing merged OTU tree" << endl;
		OTUSet otuSeen;
		for(size_t i = 0; i < otuMerged.numOTUs(); ++i) {
			PTUnrooted::PTUNodePtr node = ptu.getNode(boost::lexical_cast<size_t>(otuMerged.getOTU(i)));
			otuSeen.insert(node);
		}
		treeOut << ptu.convertToNewickTree(PTUnrooted::getAncestors(otuSeen));
	}
}
