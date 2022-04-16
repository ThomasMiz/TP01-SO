// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include "satResult.h"

const char* satResultToString(enum SatResult satResult) {
	switch (satResult) {
		case Sat:
			return "Sat";
			
		case Unsat:
			return "Unsat";
			
		case Error:
			return "Error";
			
		default:
			return "Unknown";
	}
}