#include <stdio.h>
/* closure.c */
extern short *itemset;
extern short *itemsetend;
extern void initialize_closure(int n);
extern void closure(short *core, int n);
extern void finalize_closure(void);
/* conflicts.c */
extern char any_conflicts;
extern int expected_conflicts;
extern void initialize_conflicts(void);
extern void conflict_log(void);
extern void verbose_conflict_log(void);
extern void print_reductions(int state);
/* derives.c */
extern short **derives;
extern void set_derives(void);
extern void free_derives(void); /* unused */
/* files.c */
extern int fixed_outfiles;
extern void openfiles(void);
extern void open_extra_files(void);
extern void done(int k);
/* getargs.c */
extern int verboseflag;
extern int definesflag;
extern int debugflag;
extern int nolinesflag;
extern int noparserflag;
extern int toknumflag;
extern int rawtoknumflag;
extern char *spec_name_prefix;
extern char *spec_file_prefix;
extern void getargs(int argc, char *argv[]);
/* lalr.c */
extern void lalr(void);
/* lex.c */
extern char *token_buffer;
extern int numval;
extern void init_lex(void);
extern int skip_white_space(void);
extern void unlex(int token);
extern int lex(void);
extern int parse_percent_token(void);
/* main.c */
extern char *program_name;
extern char *printable_version(char c);
extern char *int_to_string(int i);
extern void fatal(const char *s);
extern void fatals(const char *fmt, const char *x1);
extern void warn(const char *s);
extern void warni(const char *fmt, int x1);
extern void warns(const char *fmt, const char *x1);
extern void warnss(const char *fmt, const char *x1, const char *x2);
extern void warnsss(const char *fmt, const char *x1, const char *x2, const char *x3);
extern void toomany(const char *s);
extern void berror(const char *s);
/* nullable.c */
extern char *nullable;
extern void set_nullable(void);
extern void free_nullable(void); /* unused */
/* output.c */
extern void output_headers(void);
extern void output_trailers(void);
extern void output(void);
/* print.c */
extern void terse(void);
extern void verbose(void);
/* reader.c */
extern int lineno;
extern char **tags;
extern int *user_toknums;
extern void reader(void);
extern void reader_output_yylsp(FILE *f);
/* reduce.c */
extern void reduce_grammar(void);
/* version.c */
extern const char version_string[];
/* warshall.c */
extern void RTC(unsigned *R, int n);
