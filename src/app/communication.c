#include "communication.h"

const char* satResultToString(enum SatResult satResult) {
	switch (satResult) {
		case Sat:
			return "Sat";
			
		case Unsat:
			return "Unsat";
			
		case FileOpenFailed:
			return "Couldn't open file";
			
		default:
			return "Unknown";
	}
}