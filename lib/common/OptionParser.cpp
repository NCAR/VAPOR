#include <cstdio>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <algorithm>
#include <vapor/OptionParser.h>
#include <cassert>
#include <cctype>
#include <sstream>
#include <locale>
#include <vapor/Version.h>
#ifdef WIN32
    #pragma warning(disable : 4996)
#endif

using namespace Wasp;

namespace Wasp {
bool opt_cmp(OptionParser::_OptRec_T *a, OptionParser::_OptRec_T *b) { return (string(a->option) < string(b->option)); };
}    // namespace Wasp

/*
 * convert an array of strings into a singe contiguous array of strings
 */
char *copy_create_arg_string(char **argv, int argc)
{
    char *s, *t;
    int   i, len; /* lenght of new arg string	*/

    for (len = 0, i = 0; i < argc; i++) {
        len += (int)strlen(argv[i]);
        len++; /* one for the space	*/
    }
    s = (char *)malloc(len + 1);
    assert(s != NULL);

    s = strcpy(s, argv[0]);
    for (i = 1, t = s; i < argc; i++) {
        t += strlen(t);
        *t++ = '\0';
        (void)strcpy(t, argv[i]);
    }
    return (s);
}

/*
 * convert a space seprated string to an array of contiguous strings
 */
char *fmt_opt_string(char *arg, int n)
{
    int   i;
    char *s;

    for (i = 1, s = arg; i < n; i++) {
        while (*s && !isspace(*s)) s++;

        if (!*s) { return (NULL); }
        *s++ = '\0';

        while (*s && isspace(*s)) s++;
    }
    return (arg);
}

/*
**
**	Type converters. The following functions convert string representations
**	of data into their primitive data formats. A NULL 'from' value
**	should result in a reasonable value.
**
*/

/*
 *	CvtToInt()
 *
 *	convert a ascii string to its integer value
 */
int Wasp::CvtToInt(const char *from, /* the string	*/
                   void *      to)
{
    int *iptr = (int *)to;

    if (!from) {
        *iptr = 0;
    } else if (sscanf(from, "%d", iptr) != 1) {
        return (-1);
    }
    return (1);
}

/*
 *	CvtToFloat()
 *
 *	convert a ascii string to its floating point value
 */
int Wasp::CvtToFloat(const char *from, /* the string	*/
                     void *      to)
{
    float *fptr = (float *)to;

    if (!from) {
        *fptr = 0.0;
    } else if (sscanf(from, "%f", fptr) != 1) {
        return (-1);
    }
    return (1);
}

/*
 *	CvtToDouble()
 *
 *	convert a ascii string to its floating point value
 */
int Wasp::CvtToDouble(const char *from, /* the string	*/
                      void *      to)
{
    double *dptr = (double *)to;

    if (!from) {
        *dptr = 0.0;
    } else if (sscanf(from, "%lf", dptr) != 1) {
        return (-1);
    }
    return (1);
}

/*
 *	CvtToChar()
 *
 *	convert a ascii string to a char.
 */
int Wasp::CvtToChar(const char *from, /* the string	*/
                    void *      to)
{
    char *cptr = (char *)to;

    if (!from) {
        *cptr = '\0';
    } else if (sscanf(from, "%c", cptr) != 1) {
        return (-1);
    }
    return (1);
}

/*
 *	ConvetToBoolean()
 *
 *	convert a ascii string containing either "true" or "false" to
 *	to TRUE or FALSE
 */
int Wasp::CvtToBoolean(const char *from, /* the string	*/
                       void *      to)
{
    OptionParser::Boolean_T *bptr = (OptionParser::Boolean_T *)to;

    if (!from) {
        *bptr = 0;
    } else if (strcmp("true", from) == 0) {
        *bptr = 1;
    } else if (strcmp("false", from) == 0) {
        *bptr = 0;
    } else {
        return (-1);
    }
    return (1);
}

/*
 *	CvtToString()
 *
 */
int Wasp::CvtToString(const char *from, /* the string	*/
                      void *      to)
{
    char **sptr = (char **)to;

    *sptr = (char *)from;

    return (1);
}

/*
 *	CvtToCPPStr()
 *
 */
int Wasp::CvtToCPPStr(const char *from, /* the string	*/
                      void *      to)
{
    string *sptr = (string *)to;

    *sptr = from;

    return (1);
}

/*
 *	CvtToDimension2D()
 *
 *	convert a ascii string to a dimension.
 */
int Wasp::CvtToDimension2D(const char *from, /* the string	*/
                           void *      to)
{
    OptionParser::Dimension2D_T *dptr = (OptionParser::Dimension2D_T *)to;

    if (!from) {
        dptr->nx = dptr->ny = 0;
    } else if (!((sscanf(from, "%dx%d", &(dptr->nx), &(dptr->ny)) == 2) || (sscanf(from, "%dX%d", &(dptr->nx), &(dptr->ny)) == 2))) {
        return (-1);
    }
    return (1);
}

//
//	CvtToDimension3D()
//
//	convert an ascii string to a 3D dimension.
///
int Wasp::CvtToDimension3D(const char *from, /* the string	*/
                           void *      to)
{
    OptionParser::Dimension3D_T *dptr = (OptionParser::Dimension3D_T *)to;

    if (!from) {
        dptr->nx = dptr->ny = dptr->nz = 0;
    } else if (!((sscanf(from, "%dx%dx%d", &(dptr->nx), &(dptr->ny), &(dptr->nz)) == 3) || (sscanf(from, "%dX%dX%d", &(dptr->nx), &(dptr->ny), &(dptr->nz)) == 3))) {
        return (-1);
    }
    return (1);
}

//
//	CvtToStrVec()
//
//	convert a colon delimited ascii string to vector of C++ STL strings
//
int Wasp::CvtToStrVec(const char *from, /* the string	*/
                      void *      to)
{
    vector<string> *vptr = (vector<string> *)to;
    vptr->clear();

    (void)SplitString(from, ':', *vptr);

    return (1);
}
//
//	CvtToIntVec()
//
//	convert a colon delimited ascii string to vector of C++ STL ints
//
int Wasp::CvtToIntVec(const char *from, /* the string	*/
                      void *      to)
{
    vector<int> *vptr = (vector<int> *)to;
    vptr->clear();

    string s(from);
    char   delim;
    if (s.find(":") != string::npos) {
        delim = ':';
    } else {
        delim = 'x';
    }
    (void)SplitString(from, delim, *vptr);

    return (1);
}

//
//	CvtToSize_tVec()
//
//	convert a colon delimited ascii string to vector of C++ STL size_t
//
int Wasp::CvtToSize_tVec(const char *from, /* the string	*/
                         void *      to)
{
    vector<size_t> *vptr = (vector<size_t> *)to;
    vptr->clear();

    string s(from);
    char   delim;
    if (s.find(":") != string::npos) {
        delim = ':';
    } else {
        delim = 'x';
    }
    (void)SplitString(from, delim, *vptr);

    return (1);
}

//
//	CvtToFloatVec()
//
//	convert a colon delimited ascii string to vector of C++ STL float
//
int Wasp::CvtToFloatVec(const char *from, /* the string	*/
                        void *      to)
{
    vector<float> *vptr = (vector<float> *)to;
    vptr->clear();

    string s(from);
    char   delim;
    if (s.find(":") != string::npos) {
        delim = ':';
    } else {
        delim = 'x';
    }
    (void)SplitString(from, delim, *vptr);

    return (1);
}

//
//	CvtToDoubleVec()
//
//	convert a colon delimited ascii string to vector of C++ STL float
//
int Wasp::CvtToDoubleVec(const char *from, /* the string	*/
                         void *      to)
{
    vector<double> *vptr = (vector<double> *)to;
    vptr->clear();

    string s(from);
    char   delim;
    if (s.find(":") != string::npos) {
        delim = ':';
    } else {
        delim = 'x';
    }
    (void)SplitString(from, delim, *vptr);

    return (1);
}

//
//	CvtToIntRange()
//
//	convert a colon-delimited string of ints to a min/max pair
///
int Wasp::CvtToIntRange(const char *from, /* the string	*/
                        void *      to)
{
    OptionParser::IntRange_T *dptr = (OptionParser::IntRange_T *)to;

    if (!from) {
        dptr->min = dptr->max = 0;
    } else if (!((sscanf(from, "%d:%d", &(dptr->min), &(dptr->max))) == 2)) {
        return (-1);
    }
    return (1);
}

OptionParser::OptionParser() { _optTbl.clear(); }

OptionParser::~OptionParser()
{
    _OptRec_T *o;

    for (int i = 0; i < (int)_optTbl.size(); i++) {
        o = _optTbl[i];
        if (o->option) free((void *)o->option);
        if (o->value) free((void *)o->value);
        if (o->default_value) free((void *)o->default_value);
        if (o->help) free((void *)o->help);
        delete o;
    }
    _optTbl.clear();
}

/*
 *	AppendOptions
 *
 *	Add a list of valid application options to the option table. All
 *	options are assumed to begin with a '-' and contain 0 or more arguments.
 *
 *	The fields of the DPOptDescRec struct are as follows:
 *
 *	char	*option;	name of option without preceeding '-'
 *	int	arg_count;	num args expected by option
 *	char	*value;		default value for the argument - if 'arg_count'
 *				is zero 'value' is ignored. if 'arg_count' is
 *				greater than 1 'value' is a string of space
 *				separted arguments.
 *	char	*help;		help string for option
 *
 *
 * on entry
 *	od		: option descriptor
 *	optd		: Null terminated list of options
 *
 * on exit
 *	return		: -1 => failure, else OK.
 */
int OptionParser::AppendOptions(const OptDescRec_T *odr)
{
    int        i, j;
    int        num;
    _OptRec_T *opt_rec;

    if (!odr[0].option) return (0);

    /*
     * make sure there are no duplicate names in the table. This only
     * compares new entries with old entries. If there are duplicate
     * entries within the new entries they won't be found. Count
     * how many options there are as well
     */
    for (j = 0, num = 0; odr[j].option; j++) {
        for (i = 0; i < (int)_optTbl.size(); i++) {
            if (strcmp(_optTbl[i]->option, odr[j].option) == 0) {
                SetErrMsg("Option %s already in option table", odr[j].option);
                return (-1); /* duplicate entry	*/
            }
        }
        num++;
    }

    /*
     * copy all the options into the option table allocating memory
     * as necessary.
     */
    for (i = 0; i < num; i++) {
        opt_rec = new _OptRec_T;

        _optTbl.push_back(opt_rec);

        char *value = NULL;

        if (odr[i].arg_count == 0) {
            value = strdup("false");
        } else {
            if (odr[i].value) value = strdup(odr[i].value);
        }

        if (!odr[i].option) {
            SetErrMsg("Invalid option descriptor");
            return (-1);
        }

        if (value) {
            value = fmt_opt_string(value, odr[i].arg_count);
            if (!value) {
                SetErrMsg("Arg string invalid: %s", value);
                return (-1);
            }
        }

        opt_rec->option = strdup(odr[i].option);
        opt_rec->default_value = value;
        opt_rec->value = strdup(value);
        opt_rec->help = strdup(odr[i].help);
        opt_rec->arg_count = odr[i].arg_count;
    }

    return (0);
}

/*
 *	RemoveOptions
 *	[exported]
 *
 *	Remove a list of application options from the option table.
 *	Only the 'option' field of the DPOptDescRec struct is referenced,
 *	all other fields are ignored. The options to be freed must have
 *	previously been set with a call to DPLoadOptionTable().
 *
 *
 * on entry
 *	od		: option descriptor
 *	odr		: Null terminated list of options
 *
 */
void OptionParser::RemoveOptions(vector<string> options)
{
    int                           j;
    vector<_OptRec_T *>::iterator itr;

    /*
     * look for the option in the option table.
     */
    for (j = 0; j < options.size(); j++) {
        for (itr = _optTbl.begin(); itr != _optTbl.end(); ++itr) {
            _OptRec_T *o = *itr;
            if (options[j].compare(o->option) == 0) {
                if (o->option) free((void *)o->option);
                if (o->value) free((void *)o->value);
                if (o->default_value) free((void *)o->default_value);
                if (o->help) free((void *)o->help);
                _optTbl.erase(itr);
                break;
            }
        }
    }
}

/*
 *	ParseOption
 *	[exported]
 *
 *	parse the option table with the command line options. After the
 *	option table is created with DPLoadOptionTable this function may
 *	be called to change values of options in the table. We assume
 *	argv[0] is the program name and we leave it unmolested
 *
 *	DPErrLog() is invoked on error.
 *
 * on entry:
 *	**argv		: list of command line args
 *	*argc		: num elements in argv
 *	*optds		: additional options to merge into the option table
 * on exit
 *	**argv		: contains options not found in the option table
 *	*argc		: num elements in argv
 *	return		: -1 => failure, else OK
 */
int OptionParser::ParseOptions(int *argc, char **argv, Option_T *options)
{
    int        i;
    int        new_argc = 1;
    char **    next = argv + 1;
    _OptRec_T *o;

    if (!argv) return (1);

    /*
    **	Restore default values for options in option table
    */
    for (i = 0; i < (int)_optTbl.size(); i++) {
        o = _optTbl[i];
        if (o->value) free((void *)o->value);
        o->value = strdup(o->default_value);
    }

    /*
     * look for matches between elements in argv and in the option table
     */
    for (i = 1; i < *argc; i++) {
        if (*argv[i] == '-') { /* is it an option specifier?	*/
            o = _get_option((char *)(argv[i] + 1));
        } else {
            o = (_OptRec_T *)NULL; /* not a specifier */
        }

        if (o == (_OptRec_T *)-1) {
            SetErrMsg("Ambiguous option: %s", argv[i]);
            return (-1);
        }

        /*
         * if no match found leave option in argv along with anything
         * that follows that is not an option specifier
         */
        if (!o) { /* not found	*/
            *next = argv[i];
            new_argc++;
            next++;
            while (i + 1 < *argc && *argv[i + 1] != '-') {
                i++;
                new_argc++;
                *next = argv[i];
                next++;
            }
            continue;
        }

        /*
         * make sure enough args for option
         */
        if ((i + o->arg_count) >= *argc) {
            SetErrMsg("Option -%s expects %d args", o->option, o->arg_count);
            return (-1);
        }

        /*
         * Options with no args are a special case. Assign them
         * a value of true. They are false by default
         */
        if (o->arg_count == 0) {
            o->value = strdup("true");
            continue;
        }

        /*
         * convert the arg list to a single string and stash it
         * in the option table
         */
        o->value = copy_create_arg_string(&argv[i + 1], o->arg_count);
        i += o->arg_count;
    }
    *argc = new_argc;
    argv[*argc] = NULL;

    return (_parse_options(options));

    return (0);
}

/*
 *	DPParseEnvOptions()
 *
 *	DPParseEnvOptions() is analogous to DPParseOptionTable except that
 *	it takes a list of environment variables and their coresponding
 *	option names instead of an argv list. If a given environment
 *	variable is set its value is used as the argument value for the
 *	option with  which it is associated. If the environment variable
 *	is not set the option/environemnt variable pair are ignored.
 *
 * on entry
 *	*envv		: NUll-terminated list of option/env pairs
 *	*optds		: additional options to merge into the option table
 *
 * on exit
 *	return		: -1 => error, else OK
 */
int OptionParser::ParseOptions(const EnvOpt_T *envv, Option_T *opts)
{
    const EnvOpt_T *envptr;     /* pointer to envv		*/
    char **         argv;       /* arg vector created from envv	*/
    int             argc;       /* size of argv list		*/
    char *          arg_string; /* env variable value		*/
    int             i;

    // count the args
    for (envptr = envv, argc = 0; envptr->option; envptr++, argc += 2)
        ;

    argv = new char *[argc + 2];
    argv[0] = strdup("place_holder");

    /*
     * look for environment variables. Generate the argument vector, argv
     */

    for (envptr = envv, i = 1; envptr->option; envptr++, i += 2) {
        if ((arg_string = getenv(envptr->env_var))) {
            argv[i] = new char[strlen(arg_string + 2)];
            argv[i] = strcpy(argv[i], "-");
            argv[i] = strcat(argv[i], envptr->option);
            argv[i + 1] = strdup(arg_string);
        }
    }
    argv[i] = NULL;

    return (ParseOptions(&argc, argv, opts));
}

//
//	PrintOptionHelp()
//
//	Print help about each option.
//
//	Each option known to the option table is printed, followed by
//	the string "arg0 arg1 ... argN", where N+1 is the number of arguments
//	expected by the option, followed by the contents of the 'help' field.
//
// on entry
//	*fp		: file pointer where output is written.
//
void OptionParser::PrintOptionHelp(FILE *fp, int linelimit, bool docopyright)
{
    std::sort(_optTbl.begin(), _optTbl.end(), opt_cmp);

    string h1("OPTION");
    string d1("------");
    string h2("NUM_ARGS");
    string d2("--------");
    string h3("DEFAULT");
    string d3("-------");
    int    f1 = h1.size();
    int    f2 = h2.size();
    int    f3 = h3.size();
    for (int i = 0; i < (int)_optTbl.size(); i++) {
        string option = _optTbl[i]->option;
        if (option.size() > f1) f1 = option.size();

        string default_value = _optTbl[i]->default_value;
        if (default_value.size() > f3) f3 = default_value.size();
    }
    f1 += 4;
    f2 += 4;
    f3 += 4;
    ostringstream format;
    format << "    ";
    format << "%-";
    format << f1;
    format << ".";
    format << f1;
    format << "s";
    format << "%-";
    format << f2;
    format << ".";
    format << f2;
    format << "s";
    format << "%-";
    format << f3;
    format << ".";
    format << f3;
    format << "s";

    fprintf(fp, format.str().c_str(), h1.c_str(), h2.c_str(), h3.c_str());
    fprintf(fp, "\n");
    fprintf(fp, format.str().c_str(), d1.c_str(), d2.c_str(), d3.c_str());
    fprintf(fp, "\n");

    string margin("        ");
    for (int i = 0; i < (int)_optTbl.size(); i++) {
        _OptRec_T *   o = _optTbl[i];
        ostringstream arg_count_str;
        arg_count_str << o->arg_count;
        string option = "-";
        option.append(o->option);
        string defvalstr(o->default_value);
        if (!defvalstr.size()) defvalstr = "\"\"";

        fprintf(fp, format.str().c_str(), option.c_str(), arg_count_str.str().c_str(), defvalstr.c_str());

        vector<string> helpvec;
        string         helpstr(o->help);
        StrToWordVec(helpstr, helpvec);

        int linecount = linelimit;
        for (int j = 0; j < helpvec.size(); j++) {
            if (linecount + helpvec[j].size() < linelimit) {
                fprintf(fp, "%s ", helpvec[j].c_str());
                linecount += helpvec[j].size() + 1;
            } else {
                linecount = 0;
                fprintf(fp, "\n%s%s ", margin.c_str(), helpvec[j].c_str());
                linecount += margin.size() + helpvec[j].size() + 1;
            }
        }
        fprintf(fp, "\n");
    }

#ifdef DEAD
    for (i = 0; i < (int)_optTbl.size(); i++) {
        _OptRec_T *o = _optTbl[i];

        sprintf(buf, "    -%-8.8s", o->option);
        if (o->arg_count < 4) {
            for (j = 0; j < o->arg_count; j++) {
                sprintf(sbf, " arg%d", j);
                if (strlen(sbf) + strlen(buf) < sizeof(buf)) {
                    (void)strcat(buf, sbf);
                } else {
                    break;
                }
            }
        } else {
            sprintf(sbf, " arg0 .. arg%d", o->arg_count - 1);
            (void)strcat(buf, sbf);
        }
        (void)fprintf(fp, buf);
        for (j = (int)strlen(buf); j < (int)sizeof(buf); j++) { putc(' ', fp); }
        if (o->help) {
            (void)fprintf(fp, "%s\n", (char *)o->help);
        } else {
            (void)fprintf(fp, "\n");
        }
    }
#endif

    if (!docopyright) return;

    fprintf(fp, "\nCopyright 2007 The National Center for Atmospheric Research\n");

    fprintf(fp, "\nVersion: %s (%s) www.vapor.ucar.edu\n", Version::GetVersionString().c_str(), Version::GetDateString().c_str());
}

//
//	get_option
//	[internal]
//
// on entry
//	*name		: name to lookup
// on exit
//	return		: if found return command obj ptr. If name is ambiguous
//			  return -1. If not found return NULL.
//
OptionParser::_OptRec_T *OptionParser::_get_option(const char *name)
{
    _OptRec_T *o;
    _OptRec_T *found;
    int        nmatches;
    int        i;

    nmatches = 0;
    found = NULL;

    for (i = 0; i < (int)_optTbl.size(); i++) {
        o = _optTbl[i];

        if (strcmp(o->option, name) == 0) {
            return (o);    // exact match
        }

        if (strncmp(o->option, name, strlen(name)) == 0) {
            nmatches++;
            found = o;
        };
    }

    if (nmatches > 1) /* ambiguous	*/
        return ((_OptRec_T *)-1);

    return (found);
}

//
//
//	_parse_options
//
//	The fields of the Option struct are as follows:
//
//	char		*option_name;	the options name - used to look the
//					requested option in the option table.
//	int		(*type_conv)();	option type converter - converts the
//					string representation of the option
//					value into a specified format and store
//					the result at address given by 'offset'
//	void		*offset;	offset of return address
//	int		size;		size of option in bytes - if there are
//					multiple arguments for a single option
//					their destination address is computed
//					by adding 'size' to 'offset' as
//					appropriate.
//
//
// on entry
//	options		: Null terminated list of options to be returned
//
// on exit
//	options		: offset field of options record filled in
//	return		: -1 => failure, else OK
//
int OptionParser::_parse_options(const Option_T *options)
{
    int         i, j;
    const char *s;
    int         arg_count;

    _OptRec_T *o;
    void *     offset;

    for (i = 0; options[i].option_name; i++) {
        /*
         * find the option */
        o = _get_option(options[i].option_name);
        if (o == (_OptRec_T *)-1) {
            SetErrMsg("Ambiguous option: %s", options[i].option_name);
            return (-1);
        }

        if (!o) {
            //			SetErrMsg("Option %s unknown", options[i].option_name);
            //			return(-1);
            continue;
        }

        /*
         * zero arg_count options really do have a single argument
         */
        arg_count = o->arg_count ? o->arg_count : 1;

        offset = options[i].offset;
        for (j = 0, s = o->value; j < arg_count; j++) {
            if (options[i].type_conv(s, offset) < 0) {
                SetErrMsg("Type converter for option \"-%s\" "
                          "failed to convert argument \"%s\"",
                          options[i].option_name, s);
                return (-1);
            }
            if (s) s += strlen(s) + 1;
            offset = (char *)offset + options[i].size;
        }
    }
    return (0);
}
