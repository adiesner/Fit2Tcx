/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * Fit2Tcx
 * Copyright (C) Andreas Diesner 2013 <garminplugin [AT] andreas.diesner [DOT] de>
 *
 * Fit2Tcx is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fit2Tcx is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include "fit/fitReader.hpp"
#include "fit/fitFileException.hpp"

#include "fit/fitMsg_File_ID.hpp"
#include "fit/fitMsg_File_Creator.hpp"
#include "Fit2TcxConverter.h"

#include <string.h>
#include <getopt.h>

using namespace std;

const string version = "0.4";

void printUsage(string error, bool extended) {
	if (error.length() > 0) {
		std::cout << error << std::endl;
		std::cout << std::endl;
	}

	if (extended) {
		std::cout <<  "Version: fit2tcx " << version << " http://www.andreas-diesner.de/garminplugin" << std::endl;
		std::cout <<  "Copyright: Copyright (C) 2013 Andreas Diesner" << std::endl;
		std::cout <<  "License: GPL3 (or later)" << std::endl;
		std::cout <<  std::endl;
		std::cout <<  "Converts a fit workout file into a tcx file." << std::endl;
		std::cout <<  "Output is either a file or stdout." << std::endl;
		std::cout <<  std::endl;
	}
	std::cout << "Usage: fit2tcx --convert <INPUTFILE> <INPUTFILE2> <INPUTFILE3> ..." << std::endl;
	std::cout << "\t--convert <FILES> - convert multiple fit files. Output is <FILENAME>.tcx" << std::endl;
	std::cout << std::endl;
	std::cout << "Usage: fit2tcx -i <INPUTFILE> [-o <OUTPUTFILE>]" << std::endl;
	std::cout << "\t-i <INPUTFILE> - fit file to convert" << std::endl;
	std::cout << "\t-o <OUTPUTFILE> - tcx file to write (if not specified, stdout will be used)" << std::endl;
	std::cout << "\t--help - output usage information" << std::endl;
	std::cout << "\t--version - output version and usage information" << std::endl;
	std::cout << std::endl;
}


int main(int argc, char* argv[])
{
	/*
	 * Make sure that numbers are outputted 1234.56 and not 1.234,56
	 */
	setlocale(LC_ALL, "POSIX");

	bool doPrintHelp=false;
    bool doPrintHelpExtended=false;
    bool doMultipleConversions=false;
    vector<string> inputFiles;
    string outputFile="";
    int c;
    while (1) {
        static struct option long_options[] =
          {
            /* These options set a flag. */
            {"help",    no_argument, 0, 'h'},
            {"version", no_argument, 0, 'v'},
            {"input",  required_argument, 0, 'i'},
            {"output", required_argument, 0, 'o'},
            {"convert", no_argument, 0, 'c'},
            {0, 0, 0, 0}
          };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long (argc, argv, "hi:o:c:",
                         long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
          break;

        switch (c)
          {
          case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
              break;
            printf ("option %s", long_options[option_index].name);
            if (optarg)
              printf (" with arg %s", optarg);
            printf ("\n");
            break;

          case 'h': // print help
        	  doPrintHelp = true;
          break;

          case 'v': // print help & version
        	  doPrintHelp = true;
        	  doPrintHelpExtended = true;
          break;

          case 'i':// Input file
        	  inputFiles.push_back(optarg);
          break;

          case 'c':// Input file list
        	  doMultipleConversions = true;
          break;

          case 'o': // Output file
        	  outputFile = optarg;
            break;

          case '?':
            /* getopt_long already printed an error message. */
        	  printUsage("Unknown parameter", false);
        	  return 1;
          break;

          default:
            break;
          }
      }

    if (doMultipleConversions) {
        for (int i = optind; i < argc; i++) {
        	inputFiles.push_back(argv[i]);
        }
    } else {
    	if (optind < argc) {
    		string unknownParams = "";
            for (int i = optind; i < argc; i++) {
            	unknownParams += string(argv[i]) + " ";
            }
            printUsage("Unknown parameter: "+unknownParams, false);
            return 1;
    	}
    }

	if (doPrintHelp) {
	     printUsage("", doPrintHelpExtended);
	     return 0;
	}

	if (inputFiles.empty()) {
	     printUsage("Input file missing", false);
	     return 1;
	}

	bool errorExit = false;

	for(unsigned int t=0;t<inputFiles.size();++t){
		string inputFile = inputFiles.at(t);
	    Fit2TcxConverter *conv = new Fit2TcxConverter();
	    bool convertError = false;

	    if (doMultipleConversions) {
	    	outputFile = inputFile;
	    	unsigned found = outputFile.find_last_of(".");
	    	if (found != string::npos) {
	    		outputFile = outputFile.substr(0,found) + ".tcx";
	    	} else {
	    		outputFile += ".tcx";
	    	}
	    }

	    FitReader *fit = new FitReader(inputFile);
	    fit->registerFitMsgFkt(conv);
	    try {
	        if (fit->isFitFile()) {
	            int i = 0;
	            while (fit->readNextRecord())  {
	                i++;
	            }

	            fit->closeFitFile();
	        } else {
	        	std::cout << "Not a fit file: " << inputFile << std::endl;
	        	convertError = true;
	        }
	    } catch (FitFileException &e) {
	    	std::cout << "Exception: " << e.getError() << " (" << inputFile << ")" << std::endl;
	    	convertError = true;
	    } catch (...) {
	    	std::cout << "Unknown exception happened while parsing fit file! (" << inputFile << ")" << std::endl;
	    	convertError=true;
	    }
	    delete (fit);
	    fit = NULL;

	    if (convertError) {
	    	errorExit = true;
		    delete(conv);
		    conv = NULL;
	    }

	    if (conv != NULL) {
			string xml = conv->getTcxContent(true, "");
			if (!outputFile.empty()) {
				std::ofstream workoutFile;
				workoutFile.open(outputFile.c_str());
				if (workoutFile.is_open()) {
					workoutFile << xml;
					workoutFile.close();
					std::cout << "Saved to " << outputFile << std::endl;
				} else {
					std::cout << "Unable to open " << outputFile << " for writing."<< std::endl;
				}
			} else {
				std::cout << xml << std::endl;
			}

			delete(conv);
			conv = NULL;
	    }
	}

    if (errorExit) {
    	return 1;
    }
    return 0;
}
