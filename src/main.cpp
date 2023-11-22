#include "compile.hpp"

#include "compile.hpp"
#include "ast.hpp"

#include <cstdlib>
#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: ./main <test name>\n");
		return EXIT_FAILURE;
	}

	using namespace Ast;

	char const* test_name = argv[1];

	int local_var_alloc = 0;

	if (!strcmp(test_name, "fibrec")) {
		int arg1 = local_var_alloc++;

		auto fun = Func{
			"fibrec", 1, 1,
			new IfElse{
				new Var{arg1},
				new IfElse{
					new Add{new Var{arg1}, new Num{-1}},
					new Return{new Add{
						new Call{"fibrec", {new Add{new Var{arg1}, new Num{-1}}}},
						new Call{"fibrec", {new Add{new Var{arg1}, new Num{-2}}}}
					}},
					new Return{new Num{1}}
				},
				new Return{new Num{0}}
			}
		};

		compile(fun);

	} else if (!strcmp(test_name, "swap")) {

		int arg1 = local_var_alloc++;
		int arg2 = local_var_alloc++;
		int temp = local_var_alloc++;

		auto fun = Func{
			"swap",
			3, 2,
			new Seq{new Seq{new Seq{
			new Assignment{new Var{temp}, new Deref{new Var{arg1}}},
			new Assignment{new Deref{new Var{arg1}}, new Deref{new Var{arg2}}}},
			new Assignment{new Deref{new Var{arg2}}, new Var{temp}}},
			new Return{new Num{0}}}
		};

		compile(fun);


	} else if (!strcmp(test_name, "increment")) {
		int arg = local_var_alloc++;

		auto fun = Func{
			"increment",
			1, 1,
			new Return{new Add{new Num{1}, new Var{arg}}}
		};

		compile(fun);
	} else if (!strcmp(test_name, "fib")) {

		int arg = local_var_alloc++;
		int a = local_var_alloc++;
		int b = local_var_alloc++;
		int c = local_var_alloc++;

		// fun fib(arg)
		//   a = 0
		//   b = 1
		//   while arg
		//   do
		//     c = a + b
		//     a = b
		//     b = c
		//     arg = arg + (-1)
		//   end
		//   return b
		// end
		auto fun = Func{
			"fib",
			4, 1,
			new Seq{new Seq{new Seq{
				new Assignment{a, new Num{0}},
				new Assignment{b, new Num{1}}},
			new  While{
				new Var{arg},
				new Seq{new Seq{new Seq{
					new Assignment{c, new Add{new Var{a}, new Var{b}}},
					new Assignment{a, new Var{b}}},
				new Assignment{b, new Var{c}}},
				new Assignment{arg, new Add{new Var{arg}, new Num{-1}}}}
			}},
			new Return{new Var{b}}}
		};

		compile(fun);

	} else if (!strcmp(test_name, "read_next_value")) {

		int arg = local_var_alloc++;
#if 1
		int nxt = local_var_alloc++;
		int nxt_val = local_var_alloc++;
		// nxt = *(arg + 8)
		// nxt_val = *nxt
		// return nxt_val
		auto fun = Func{
			"read_next_value",
			3, 1,
			new Seq{new Seq{
				new Assignment{nxt, new Deref{new Add{new Var{arg}, new Num{8}}}},
				new Assignment{nxt_val, new Deref{new Var{nxt}}}},
			new Return{new Var{nxt_val}}}
		};
#else

		auto fun = Func{
			"read_next_value",
				new Return{new Deref{new Deref{new Add{new Var{arg}, new Num{8}}}}}
		};

#endif
		compile(fun);

	} else {
		fprintf(stderr, "Invalid test name: '%s'\n", test_name);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
