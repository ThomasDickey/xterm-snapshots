dnl
dnl $XFree86: xc/programs/xterm/aclocal.m4,v 3.12 1998/03/27 23:23:58 hohndel Exp $
dnl
dnl ---------------------------------------------------------------------------
dnl 
dnl Copyright 1997,1998 by Thomas E. Dickey <dickey@clark.net>
dnl 
dnl                         All Rights Reserved
dnl 
dnl Permission to use, copy, modify, and distribute this software and its
dnl documentation for any purpose and without fee is hereby granted,
dnl provided that the above copyright notice appear in all copies and that
dnl both that copyright notice and this permission notice appear in
dnl supporting documentation, and that the name of the above listed
dnl copyright holder(s) not be used in advertising or publicity pertaining
dnl to distribution of the software without specific, written prior
dnl permission.
dnl 
dnl THE ABOVE LISTED COPYRIGHT HOLDER(S) DISCLAIM ALL WARRANTIES WITH REGARD
dnl TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
dnl AND FITNESS, IN NO EVENT SHALL THE ABOVE LISTED COPYRIGHT HOLDER(S) BE
dnl LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
dnl WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
dnl ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
dnl OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
dnl 
dnl ---------------------------------------------------------------------------
dnl ---------------------------------------------------------------------------
dnl This is adapted from the macros 'fp_PROG_CC_STDC' and 'fp_C_PROTOTYPES'
dnl in the sharutils 4.2 distribution.
AC_DEFUN([CF_ANSI_CC_CHECK],
[
AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(cf_cv_ansi_cc,[
cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" \
	"" \
	-qlanglvl=ansi \
	-std1 \
	"-Aa -D_HPUX_SOURCE +e" \
	"-Aa -D_HPUX_SOURCE" \
	-Xc
do
	CFLAGS="$cf_save_CFLAGS $cf_arg"
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || __STDC__ != 1
choke me
#endif
#endif
],[
	int test (int i, double x);
	struct s1 {int (*f) (int a);};
	struct s2 {int (*f) (double a);};],
	[cf_cv_ansi_cc="$cf_arg"; break])
done
CFLAGS="$cf_save_CFLAGS"
])
AC_MSG_RESULT($cf_cv_ansi_cc)

if test "$cf_cv_ansi_cc" != "no"; then
if test ".$cf_cv_ansi_cc" != ".-DCC_HAS_PROTOS"; then
	CFLAGS="$CFLAGS $cf_cv_ansi_cc"
else
	AC_DEFINE(CC_HAS_PROTOS)
fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Allow user to disable a normally-on option.
AC_DEFUN([CF_ARG_DISABLE],
[CF_ARG_OPTION($1,[$2 (default: on)],[$3],[$4],yes)])dnl
dnl ---------------------------------------------------------------------------
dnl Allow user to enable a normally-off option.
AC_DEFUN([CF_ARG_ENABLE],
[CF_ARG_OPTION($1,[$2 (default: off)],[$3],[$4],no)])dnl
dnl ---------------------------------------------------------------------------
dnl Restricted form of AC_ARG_ENABLE that ensures user doesn't give bogus
dnl values.
dnl
dnl Parameters:
dnl $1 = option name
dnl $2 = help-string
dnl $3 = action to perform if option is not default
dnl $4 = action if perform if option is default
dnl $5 = default option value (either 'yes' or 'no')
AC_DEFUN([CF_ARG_OPTION],
[AC_ARG_ENABLE($1,[$2],[test "$enableval" != ifelse($5,no,yes,no) && enableval=ifelse($5,no,no,yes)
  if test "$enableval" != "$5" ; then
ifelse($3,,[    :]dnl
,[    $3]) ifelse($4,,,[
  else
    $4])
  fi],[enableval=$5 ifelse($4,,,[
  $4
])dnl
  ])])dnl
dnl ---------------------------------------------------------------------------
dnl Check if we're accidentally using a cache from a different machine.
dnl Derive the system name, as a check for reusing the autoconf cache.
dnl
dnl If we've packaged config.guess and config.sub, run that (since it does a
dnl better job than uname). 
AC_DEFUN([CF_CHECK_CACHE],
[
if test -f $srcdir/config.guess ; then
	AC_CANONICAL_HOST
	system_name="$host_os"
else
	system_name="`(uname -s -r) 2>/dev/null`"
	if test -z "$system_name" ; then
		system_name="`(hostname) 2>/dev/null`"
	fi
fi
test -n "$system_name" && AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name")
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])

test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT("Configuring for $cf_cv_system_name")

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT(Cached system name ($system_name) does not agree with actual ($cf_cv_system_name))
	AC_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for data that is usually declared in <stdio.h> or <errno.h>
dnl $1 = the name to check
AC_DEFUN([CF_CHECK_ERRNO],
[
AC_MSG_CHECKING([declaration of $1])
AC_CACHE_VAL(cf_cv_dcl_$1,[
    AC_TRY_COMPILE([
#if HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdio.h>
#include <sys/types.h>
#include <errno.h> ],
    [long x = (long) $1],
    [eval 'cf_cv_dcl_'$1'=yes'],
    [eval 'cf_cv_dcl_'$1'=no]')])
eval 'cf_result=$cf_cv_dcl_'$1
AC_MSG_RESULT($cf_result)

# It's possible (for near-UNIX clones) that the data doesn't exist
AC_CACHE_VAL(cf_cv_have_$1,[
if test $cf_result = no ; then
    eval 'cf_result=DECL_'$1
    CF_UPPER(cf_result,$cf_result)
    AC_DEFINE_UNQUOTED($cf_result)
    AC_MSG_CHECKING([existence of $1])
        AC_TRY_LINK([
#undef $1
extern long $1;
],
            [$1 = 2],
            [eval 'cf_cv_have_'$1'=yes'],
            [eval 'cf_cv_have_'$1'=no'])
        eval 'cf_result=$cf_cv_have_'$1
        AC_MSG_RESULT($cf_result)
else
    eval 'cf_cv_have_'$1'=yes'
fi
])
eval 'cf_result=HAVE_'$1
CF_UPPER(cf_result,$cf_result)
eval 'test $cf_cv_have_'$1' = yes && AC_DEFINE_UNQUOTED($cf_result)'
])dnl
dnl ---------------------------------------------------------------------------
dnl You can always use "make -n" to see the actual options, but it's hard to
dnl pick out/analyze warning messages when the compile-line is long.
dnl
dnl Sets:
dnl	ECHO_LD - symbol to prefix "cc -o" lines
dnl	RULE_CC - symbol to put before implicit "cc -c" lines (e.g., .c.o)
dnl	SHOW_CC - symbol to put before explicit "cc -c" lines
dnl	ECHO_CC - symbol to put before any "cc" line
dnl
AC_DEFUN([CF_DISABLE_ECHO],[
AC_MSG_CHECKING(if you want to see long compiling messages)
CF_ARG_DISABLE(echo,
	[  --disable-echo          test: display "compiling" commands],
	[
    ECHO_LD='@echo linking [$]@;'
    RULE_CC='	@echo compiling [$]<'
    SHOW_CC='	@echo compiling [$]@'
    ECHO_CC='@'
],[
    ECHO_LD=''
    RULE_CC='# compiling'
    SHOW_CC='# compiling'
    ECHO_CC=''
])
AC_MSG_RESULT($enableval)
AC_SUBST(ECHO_LD)
AC_SUBST(RULE_CC)
AC_SUBST(SHOW_CC)
AC_SUBST(ECHO_CC)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if 'errno' is declared in <errno.h>
AC_DEFUN([CF_ERRNO],
[
CF_CHECK_ERRNO(errno)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for memmove, or a bcopy that can handle overlapping copy.  If neither
dnl is found, add our own version of memmove to the list of objects.
AC_DEFUN([CF_FUNC_MEMMOVE],
[
if test ".$ac_cv_func_memmove" != .yes ; then
	if test ".$ac_cv_func_bcopy" = ".yes" ; then
		AC_MSG_CHECKING(if bcopy does overlapping moves)
		AC_CACHE_VAL(cf_cv_good_bcopy,[
			AC_TRY_RUN([
int main() {
	static char data[] = "abcdefghijklmnopqrstuwwxyz";
	char temp[40];
	bcopy(data, temp, sizeof(data));
	bcopy(temp+10, temp, 15);
	bcopy(temp+5, temp+15, 10);
	exit (strcmp(temp, "klmnopqrstuwwxypqrstuwwxyz"));
}
		],
		[cf_cv_good_bcopy=yes],
		[cf_cv_good_bcopy=no],
		[cf_cv_good_bcopy=unknown])
		])
		AC_MSG_RESULT($cf_cv_good_bcopy)
	else
		cf_cv_good_bcopy=no
	fi
	if test $cf_cv_good_bcopy = yes ; then
		AC_DEFINE(USE_OK_BCOPY)
	else
		AC_DEFINE(USE_MY_MEMMOVE)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for tgetent function in termcap library.  If we cannot find this,
dnl we'll use the $LINES and $COLUMNS environment variables to pass screen
dnl size information to subprocesses.  (We cannot use terminfo's compatibility
dnl function, since it cannot provide the termcap-format data).
AC_DEFUN([CF_FUNC_TGETENT],
[
AC_CACHE_CHECK(for full tgetent function,cf_cv_lib_tgetent,[
cf_save_LIBS="$LIBS"
cf_cv_lib_tgetent=no
cf_TERMLIB="termcap termlib ncurses curses"
for cf_termlib in $cf_TERMLIB ; do
	LIBS="$cf_save_LIBS -l$cf_termlib"
	AC_TRY_RUN([
/* terminfo implementations ignore the buffer argument, making it useless for
 * the xterm application, which uses this information to make a new TERMCAP
 * environment variable.
 */
int main()
{
	char buffer[1024];
	buffer[0] = 0;
	tgetent(buffer, "vt100");
	exit(buffer[0] == 0); }],
	[echo "yes, there is a termcap/tgetent in $cf_termlib" 1>&AC_FD_CC
	 cf_cv_lib_tgetent="-l$cf_termlib"
	 break],
	[echo "no, there is no termcap/tgetent in $cf_termlib" 1>&AC_FD_CC],
	[echo "cross-compiling, cannot verify if a termcap/tgetent is present in $cf_termlib" 1>&AC_FD_CC])
done
LIBS="$cf_save_LIBS"
])

# If we found a working tgetent(), set LIBS and check for termcap.h.
# (LIBS cannot be set inside AC_CACHE_CHECK; the commands there should
# not have side effects other than setting the cache variable, because
# they are not executed when a cached value exists.)
if test $cf_cv_lib_tgetent != no ; then
	LIBS="$LIBS $cf_cv_lib_tgetent"
	AC_CHECK_HEADERS(termcap.h)
else
        # If we didn't find a tgetent() that supports the buffer
        # argument, look again to see whether we can find even
        # a crippled one.  A crippled tgetent() is still useful to
        # validate values for the TERM environment variable given to
        # child processes.
	AC_CACHE_CHECK(for partial tgetent function,cf_cv_lib_part_tgetent,[
	cf_cv_lib_part_tgetent=no
	for cf_termlib in $cf_TERMLIB ; do
		LIBS="$cf_save_LIBS -l$cf_termlib"
		AC_TRY_LINK([],[tgetent(0, 0)],
			[echo "there is a terminfo/tgetent in $cf_termlib" 1>&AC_FD_CC
			 cf_cv_lib_part_tgetent="-l$cf_termlib"
			 break])
	done
	LIBS="$cf_save_LIBS"
	])

	if test $cf_cv_lib_part_tgetent != no ; then
		LIBS="$LIBS $cf_cv_lib_part_tgetent"
		AC_CHECK_HEADERS(termcap.h)

                # If this is linking against ncurses, we'll trigger the
                # ifdef in resize.c that turns the termcap stuff back off.
		AC_DEFINE(USE_TERMINFO)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[
if test -n "$GCC"
then
cat > conftest.i <<EOF
#ifndef GCC_PRINTF
#define GCC_PRINTF 0
#endif
#ifndef GCC_SCANF
#define GCC_SCANF 0
#endif
#ifndef GCC_NORETURN
#define GCC_NORETURN /* nothing */
#endif
#ifndef GCC_UNUSED
#define GCC_UNUSED /* nothing */
#endif
EOF
if test -n "$GCC"
then
	AC_CHECKING([for gcc __attribute__ directives])
	changequote(,)dnl
cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
#include "confdefs.h"
#include "conftest.h"
#include "conftest.i"
#if	GCC_PRINTF
#define GCC_PRINTFLIKE(fmt,var) __attribute__((format(printf,fmt,var)))
#else
#define GCC_PRINTFLIKE(fmt,var) /*nothing*/
#endif
#if	GCC_SCANF
#define GCC_SCANFLIKE(fmt,var)  __attribute__((format(scanf,fmt,var)))
#else
#define GCC_SCANFLIKE(fmt,var)  /*nothing*/
#endif
extern void wow(char *,...) GCC_SCANFLIKE(1,2);
extern void oops(char *,...) GCC_PRINTFLIKE(1,2) GCC_NORETURN;
extern void foo(void) GCC_NORETURN;
int main(int argc GCC_UNUSED, char *argv[] GCC_UNUSED) { return 0; }
EOF
	changequote([,])dnl
	for cf_attribute in scanf printf unused noreturn
	do
		CF_UPPER(CF_ATTRIBUTE,$cf_attribute)
		cf_directive="__attribute__(($cf_attribute))"
		echo "checking for gcc $cf_directive" 1>&AC_FD_CC
		case $cf_attribute in
		scanf|printf)
		cat >conftest.h <<EOF
#define GCC_$CF_ATTRIBUTE 1
EOF
			;;
		*)
		cat >conftest.h <<EOF
#define GCC_$CF_ATTRIBUTE $cf_directive
EOF
			;;
		esac
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... $cf_attribute)
			cat conftest.h >>confdefs.h
#		else
#			sed -e 's/__attr.*/\/*nothing*\//' conftest.h >>confdefs.h
		fi
	done
else
	fgrep define conftest.i >>confdefs.h
fi
rm -rf conftest*
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally)
dnl	-pedantic
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[
if test -n "$GCC"
then
	changequote(,)dnl
	cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[]) { return argv[argc-1] == 0; }
EOF
	changequote([,])dnl
	AC_CHECKING([for gcc warning options])
	cf_save_CFLAGS="$CFLAGS"
	EXTRA_CFLAGS="-W -Wall"
	cf_warn_CONST=""
	test "$with_ext_const" = yes && cf_warn_CONST="Wwrite-strings"
	for cf_opt in \
		Wbad-function-cast \
		Wcast-align \
		Wcast-qual \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes $cf_warn_CONST
	do
		CFLAGS="$cf_save_CFLAGS $EXTRA_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			EXTRA_CFLAGS="$EXTRA_CFLAGS -$cf_opt"
			test "$cf_opt" = Wcast-qual && EXTRA_CFLAGS="$EXTRA_CFLAGS -DXTSTRINGDEFINES"
		fi
	done
	rm -f conftest*
	CFLAGS="$cf_save_CFLAGS"
fi
AC_SUBST(EXTRA_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl Use imake to obtain compiler flags.  We could, in principle, write tests to
dnl get these, but if imake is properly configured there is no point in doing
dnl this.
AC_DEFUN([CF_IMAKE_CFLAGS],
[
AC_PATH_PROGS(IMAKE,xmkmf imake)
case $IMAKE in # (vi
*/imake)
	cf_imake_opts="-DUseInstalled=YES" # (vi
	;;
*)
	cf_imake_opts=
	;;
esac

# If it's installed properly, imake (or its wrapper, xmkmf) will point to the
# config directory.
if mkdir conftestdir; then
	cd conftestdir
	echo >./Imakefile
	test -f ../Imakefile && cat ../Imakefile >>./Imakefile
	cat >> ./Imakefile <<'CF_EOF'
findstddefs:
	@echo 'IMAKE_CFLAGS="${ALLDEFINES} ifelse($1,,,$1)"'
CF_EOF
	if ( $IMAKE $cf_imake_opts 1>/dev/null 2>&AC_FD_CC && test -f Makefile)
	then
		CF_VERBOSE(Using $IMAKE)
	else
		# sometimes imake doesn't have the config path compiled in.  Find it.
		cf_config=
		for cf_libpath in $X_LIBS $LIBS ; do
			case $cf_libpath in # (vi
			-L*)
				cf_libpath=`echo .$cf_libpath | sed -e 's/^...//'`
				cf_libpath=$cf_libpath/X11/config
				if test -d $cf_libpath ; then
					cf_config=$cf_libpath
					break
				fi
				;;
			esac
		done
		if test -z "$cf_config" ; then
			AC_WARN(Could not find imake config-directory)
		else
			cf_imake_opts="$cf_imake_opts -I$cf_config"
			if ( $IMAKE -v $cf_imake_opts 2>&AC_FD_CC)
			then
				CF_VERBOSE(Using $IMAKE $cf_config)
			else
				AC_WARN(Cannot run $IMAKE)
			fi
		fi
	fi

	# GNU make sometimes prints "make[1]: Entering...", which
	# would confuse us.
	eval `make findstddefs 2>/dev/null | grep -v make`

	cd ..
	rm -rf conftestdir

	# We use $(ALLDEFINES) rather than $(STD_DEFINES) because the former
	# declares XTFUNCPROTO there.  However, some vendors (e.g., SGI) have
	# modified it to support site.cf, adding a kludge for the /usr/include
	# directory.  Try to filter that out, otherwise gcc won't find its
	# headers.
	if test -n "$GCC" ; then
	    if test -n "$IMAKE_CFLAGS" ; then
		cf_nostdinc=""
		cf_std_incl=""
		cf_cpp_opts=""
		for cf_opt in $IMAKE_CFLAGS
		do
		    case "$cf_opt" in
		    -nostdinc) #(vi
			cf_nostdinc="$cf_opt"
			;;
		    -I/usr/include) #(vi
			cf_std_incl="$cf_opt"
			;;
		    *) #(vi
			cf_cpp_opts="$cf_cpp_opts $cf_opt"
			;;
		    esac
		done
		if test -z "$cf_nostdinc" ; then
		    IMAKE_CFLAGS="$cf_cpp_opts $cf_std_incl"
		elif test -z "$cf_std_incl" ; then
		    IMAKE_CFLAGS="$cf_cpp_opts $cf_nostdinc"
		else
		    CF_VERBOSE(suppressed \"$cf_nostdinc\" and \"$cf_std_incl\")
		    IMAKE_CFLAGS="$cf_cpp_opts"
		fi
	    fi
	fi
fi
AC_SUBST(IMAKE_CFLAGS)
])dnl
dnl ---------------------------------------------------------------------------
dnl	On both Ultrix and CLIX, I find size_t defined in <stdio.h>
AC_DEFUN([CF_SIZE_T],
[
AC_MSG_CHECKING(for size_t in <sys/types.h> or <stdio.h>)
AC_CACHE_VAL(cf_cv_type_size_t,[
	AC_TRY_COMPILE([
#include <sys/types.h>
#if STDC_HEADERS
#include <stdlib.h>
#include <stddef.h>
#endif
#include <stdio.h>],
		[size_t x],
		[cf_cv_type_size_t=yes],
		[cf_cv_type_size_t=no])
	])
AC_MSG_RESULT($cf_cv_type_size_t)
test $cf_cv_type_size_t = no && AC_DEFINE(size_t, unsigned)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for the declaration of fd_set.  Some platforms declare it in
dnl <sys/types.h>, and some in <sys/select.h>, which requires <sys/types.h>
AC_DEFUN([CF_TYPE_FD_SET],
[
AC_MSG_CHECKING(for declaration of fd_set)
AC_CACHE_VAL(cf_cv_type_fd_set,[
AC_TRY_COMPILE([
#include <sys/types.h>],
	[fd_set x],
	[cf_cv_type_fd_set=sys/types.h],
	[AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/select.h>],
	[fd_set x],
	[cf_cv_type_fd_set=sys/select.h],
	[cf_cv_type_fd_set=unknown])])])
AC_MSG_RESULT($cf_cv_type_fd_set)
if test $cf_cv_type_fd_set = sys/select.h ; then
	AC_DEFINE(USE_SYS_SELECT_H)
fi
])
dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a variable
dnl $1=uppercase($2)
AC_DEFUN([CF_UPPER],
[
changequote(,)dnl
$1=`echo $2 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
])dnl
dnl ---------------------------------------------------------------------------
dnl Use AC_VERBOSE w/o the warnings
AC_DEFUN([CF_VERBOSE],
[test -n "$verbose" && echo "	$1" 1>&AC_FD_MSG
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for Xaw (Athena) libraries
dnl
AC_DEFUN([CF_X_ATHENA],
[AC_REQUIRE([CF_X_TOOLKIT])
cf_x_athena=${cf_x_athena-Xaw}

AC_ARG_WITH(Xaw3d,
	[  --with-Xaw3d            link with Xaw 3d library],
	[cf_x_athena=Xaw3d])

AC_ARG_WITH(neXtaw,
	[  --with-neXtaw           link with neXT Athena library],
	[cf_x_athena=neXtaw])

AC_CHECK_HEADERS(X11/$cf_x_athena/SimpleMenu.h)

AC_CHECK_LIB(Xmu, XmuClientWindow,,[
AC_CHECK_LIB(Xmu_s, XmuClientWindow)])

AC_CHECK_LIB(Xext,XextCreateExtension,
	[LIBS="-lXext $LIBS"])

AC_CHECK_LIB($cf_x_athena, XawSimpleMenuAddGlobalActions,
	[LIBS="-l$cf_x_athena $LIBS"],[
AC_CHECK_LIB(${cf_x_athena}_s, XawSimpleMenuAddGlobalActions,
	[LIBS="-l${cf_x_athena}_s $LIBS"],
	AC_ERROR(
[Unable to successfully link Athena library (-l$cf_x_athena) with test program]),
	[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])])
CF_UPPER(CF_X_ATHENA_LIBS,HAVE_LIB_$cf_x_athena)
AC_DEFINE_UNQUOTED($CF_X_ATHENA_LIBS)
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for X Toolkit libraries
dnl
AC_DEFUN([CF_X_TOOLKIT],
[
AC_REQUIRE([CF_CHECK_CACHE])
# We need to check for -lsocket and -lnsl here in order to work around an
# autoconf bug.  autoconf-2.12 is not checking for these prior to checking for
# the X11R6 -lSM and -lICE libraries.  The resultant failures cascade...
# 	(tested on Solaris 2.5 w/ X11R6)
SYSTEM_NAME=`echo "$cf_cv_system_name"|tr ' ' -`
cf_have_X_LIBS=no
case $SYSTEM_NAME in
changequote(,)dnl
irix[56]*) ;;
changequote([,])dnl
clix*)
	# FIXME: modify the library lookup in autoconf to
	# allow _s.a suffix ahead of .a
	AC_CHECK_LIB(c_s,open,
		[LIBS="-lc_s $LIBS"
	AC_CHECK_LIB(bsd,gethostname,
		[LIBS="-lbsd $LIBS"
	AC_CHECK_LIB(nsl_s,gethostname,
		[LIBS="-lnsl_s $LIBS"
	AC_CHECK_LIB(X11_s,XOpenDisplay,
		[LIBS="-lX11_s $LIBS"
	AC_CHECK_LIB(Xt_s,XtAppInitialize,
		[LIBS="-lXt_s $LIBS"
		 cf_have_X_LIBS=Xt
		]) ]) ]) ]) ])
	;;
*)
	AC_CHECK_LIB(socket,socket)
	AC_CHECK_LIB(nsl,gethostname)
	;;
esac

if test $cf_have_X_LIBS = no ; then
	AC_PATH_XTRA
	LDFLAGS="$LDFLAGS $X_LIBS"
	CFLAGS="$CFLAGS $X_CFLAGS"
	AC_CHECK_LIB(X11,XOpenDisplay,
		[LIBS="-lX11 $LIBS"],,
		[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])
	AC_CHECK_LIB(Xt, XtAppInitialize,
		[AC_DEFINE(HAVE_LIBXT)
		 cf_have_X_LIBS=Xt
		 LIBS="-lXt $X_PRE_LIBS $LIBS"],,
		[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])
else
	LDFLAGS="$LDFLAGS $X_LIBS"
	CFLAGS="$CFLAGS $X_CFLAGS"
fi

if test $cf_have_X_LIBS = no ; then
	AC_WARN(
[Unable to successfully link X Toolkit library (-lXt) with
test program.  You will have to check and add the proper libraries by hand
to makefile.])
fi
])dnl
