dnl
dnl $XFree86: xc/programs/xterm/aclocal.m4,v 3.1.2.3 1997/05/26 14:36:27 dawes Exp $
dnl
dnl ---------------------------------------------------------------------------
dnl 
dnl Copyright 1997 by Thomas E. Dickey <dickey@clark.net>
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
dnl This is adapted from the macros 'fp_PROG_CC_STDC' and 'fp_C_PROTOTYPES'
dnl in the sharutils 4.2 distribution.
dnl
AC_DEFUN([CF_ANSI_CC],
[AC_MSG_CHECKING(for ${CC-cc} option to accept ANSI C)
AC_CACHE_VAL(cf_cv_ansi_cc,
[cf_cv_ansi_cc=no
cf_save_CFLAGS="$CFLAGS"
# Don't try gcc -ansi; that turns off useful extensions and
# breaks some systems' header files.
# AIX			-qlanglvl=ansi
# Ultrix and OSF/1	-std1
# HP-UX			-Aa -D_HPUX_SOURCE
# SVR4			-Xc
# UnixWare 1.2		(cannot use -Xc, since ANSI/POSIX clashes)
for cf_arg in "-DCC_HAS_PROTOS" "" -qlanglvl=ansi -std1 "-Aa -D_HPUX_SOURCE" -Xc
do
	CFLAGS="$cf_save_CFLAGS $cf_arg"
	AC_TRY_COMPILE(
[
#ifndef CC_HAS_PROTOS
#if !defined(__STDC__) || __STDC__ != 1
choke me
#endif
#endif
], [int test (int i, double x);
struct s1 {int (*f) (int a);};
struct s2 {int (*f) (double a);};],
[cf_cv_ansi_cc="$cf_arg"; break])
done
CFLAGS="$cf_save_CFLAGS"
])
AC_MSG_RESULT($cf_cv_ansi_cc)
test ".$cf_cv_ansi_cc" != .no && AC_DEFINE(CC_HAS_PROTOS)
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
AC_DEFUN([CF_CHECK_CACHE],
[
system_name="`(uname -s -r) 2>/dev/null`"
if test -n "$system_name" ; then
	AC_DEFINE_UNQUOTED(SYSTEM_NAME,"$system_name")
else
	system_name="`(hostname) 2>/dev/null`"
fi
AC_CACHE_VAL(cf_cv_system_name,[cf_cv_system_name="$system_name"])
test -z "$system_name" && system_name="$cf_cv_system_name"
test -n "$cf_cv_system_name" && AC_MSG_RESULT("Configuring for $cf_cv_system_name")

if test ".$system_name" != ".$cf_cv_system_name" ; then
	AC_MSG_RESULT("Cached system name does not agree with actual")
	AC_ERROR("Please remove config.cache and try again.")
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Check for memmove, or a bcopy that can handle overlapping copy.  If neither
dnl is found, add our own version of memmove to the list of objects.
AC_DEFUN([CF_FUNC_MEMMOVE],
[
if test ".$ac_cv_func_memmove" != .yes ; then
	if test $ac_cv_func_bcopy = yes ; then
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
AC_MSG_CHECKING(for workable tgetent function)
AC_CACHE_VAL(cf_cv_func_tgetent,[
cf_save_LIBS="$LIBS"
cf_cv_func_tgetent=no
cf_TERMLIB="termcap termlib ncurses curses"
for cf_termlib in $cf_TERMLIB
do
	LIBS="$cf_save_LIBS -l$cf_termlib"
	AC_TRY_RUN([
/* terminfo implementations ignore the buffer argument, making it useless for
 * the xterm application, which uses this information to make a new $TERMCAP
 */
int main()
{
	char buffer[1024];
	buffer[0] = 0;
	tgetent(buffer, "vt100");
	exit(buffer[0] == 0); }],
	[cf_cv_func_tgetent=yes
	 break],
	[cf_cv_func_tgetent=no],
	[cf_cv_func_tgetent=no])
done
# If there was no workable (termcap) version, maybe there is a terminfo version
if test $cf_cv_func_tgetent = no ; then
	for cf_termlib in $cf_TERMLIB
	do
		AC_TRY_LINK([],[tgetent(0, 0)],
			[cf_cv_func_tgetent=$cf_termlib
			 break],
			[LIBS="$cf_save_LIBS"])
	done
fi
])
AC_MSG_RESULT($cf_cv_func_tgetent)
# If we found any sort of tgetent, check for the termcap.h file.  If this is
# linking against ncurses, we'll trigger the ifdef in resize.c that turns the
# termcap stuff back off.
if test $cf_cv_func_tgetent != no ; then
	AC_CHECK_HEADERS(termcap.h)
	if test $cf_cv_func_tgetent != yes ; then
		AC_DEFINE(USE_TERMINFO)
	fi
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Test for availability of useful gcc __attribute__ directives to quiet
dnl compiler warnings.  Though useful, not all are supported -- and contrary
dnl to documentation, unrecognized directives cause older compilers to barf.
AC_DEFUN([CF_GCC_ATTRIBUTES],
[cat > conftest.i <<EOF
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
		CF_UPPERCASE($cf_attribute,CF_ATTRIBUTE)
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

])dnl
dnl ---------------------------------------------------------------------------
dnl Check if the compiler supports useful warning options.  There's a few that
dnl we don't use, simply because they're too noisy:
dnl
dnl	-Wconversion (useful in older versions of gcc, but not in gcc 2.7.x)
dnl	-Wredundant-decls (system headers make this too noisy)
dnl	-Wtraditional (combines too many unrelated messages, only a few useful)
dnl	-Wwrite-strings (too noisy, but should review occasionally)
dnl
AC_DEFUN([CF_GCC_WARNINGS],
[cf_warn_CFLAGS=""
if test -n "$GCC"
then
	CF_GCC_ATTRIBUTES
	changequote(,)dnl
	cat > conftest.$ac_ext <<EOF
#line __oline__ "configure"
int main(int argc, char *argv[]) { return argv[argc-1] == 0; }
EOF
	changequote([,])dnl
	AC_CHECKING([for gcc warning options])
	cf_save_CFLAGS="$CFLAGS"
	cf_warn_CFLAGS="-W -Wall"
	for cf_opt in \
		Wbad-fuvction-cast \
		Wcast-align \
		Wcast-qual \
		Winline \
		Wmissing-declarations \
		Wmissing-prototypes \
		Wnested-externs \
		Wpointer-arith \
		Wshadow \
		Wstrict-prototypes
	do
		CFLAGS="$cf_save_CFLAGS $cf_warn_CFLAGS -$cf_opt"
		if AC_TRY_EVAL(ac_compile); then
			test -n "$verbose" && AC_MSG_RESULT(... -$cf_opt)
			cf_warn_CFLAGS="$cf_warn_CFLAGS -$cf_opt"
			test "$cf_opt" = Wcast-qual && cf_warn_CFLAGS="$cf_warn_CFLAGS -DXTSTRINGDEFINES"
		fi
	done
	rm -f conftest*
	CFLAGS="$cf_save_CFLAGS"
fi
])dnl
dnl ---------------------------------------------------------------------------
dnl Use imake to obtain compiler flags.  We could, in principal, write tests to
dnl get these, but if imake is properly configured there is no point in doing
dnl this.
AC_DEFUN([CF_IMAKE_CFLAGS],
[
rm -f Makefile Makefile.bak
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
if ( $IMAKE $cf_imake_opts 1>/dev/null 2>&AC_FD_CC)
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
	if test -z $cf_config ; then
		AC_ERROR(Could not find imake config-directory)
	fi
	cf_imake_opts="$cf_imake_opts -I$cf_config"
	if ( $IMAKE -v $cf_imake_opts 2>&AC_FD_CC)
	then
		CF_VERBOSE(Using $IMAKE $cf_config)
	else
		AC_ERROR(Cannot run imake)
	fi
fi

# If we've gotten this far, we have a Makefile for xterm.  Some X11R5 config
# macros do not work well enough to actually use the Makefile for a build, but
# the definitions are usable (probably).
AC_MSG_CHECKING(for compiler options known to imake)
AC_CACHE_VAL(cf_cv_imake_cflags,[
	test -n "$verbose" && echo working...
	cf_imake_cflags=`${MAKE-make} -n -f Makefile main.o RM=echo 2>/dev/null`
	for cf_opt in $cf_imake_cflags
	do
		cf_found=no
		case $cf_opt in # (vi
changequote(,)dnl
		-[focg]) cf_found=yes
			;; # (vi
		-[OID]*) #(vi
changequote([,])dnl
			for cf_opt2 in $CFLAGS
			do
				if test ".$cf_opt" = ".$cf_opt2" ; then
					cf_found=yes
					break
				fi
			done
			;;
		-*)	;; #(vi
		$CC|cc|gcc|main.*|echo)	cf_found=yes
			;;
		esac
		if test $cf_found = no ; then
			CF_VERBOSE(flag:$cf_opt)
			cf_cv_imake_cflags="$cf_cv_imake_cflags $cf_opt"
		else
			CF_VERBOSE(skip:$cf_opt)
		fi
	done
])
test -z "$verbose" && AC_MSG_RESULT(done)
IMAKE_CFLAGS="$cf_cv_imake_cflags"
rm -f Makefile Makefile.bak
])dnl
dnl ---------------------------------------------------------------------------
dnl Make an uppercase version of a given name
AC_DEFUN([CF_UPPERCASE],
[
changequote(,)dnl
$2=`echo $1 |tr '[a-z]' '[A-Z]'`
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
AC_CHECK_HEADERS(X11/Xaw/SimpleMenu.h)
AC_CHECK_LIB(Xmu, XmuClientWindow)
AC_CHECK_LIB(Xext,XextCreateExtension,[LIBS="-lXext $LIBS"])
AC_CHECK_LIB(Xaw, XawSimpleMenuAddGlobalActions, [LIBS="-lXaw $LIBS"],
	AC_ERROR(
[Unable to successfully link Athena library (-lXaw)]),
	[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])

])dnl
dnl ---------------------------------------------------------------------------
dnl Check for X Toolkit libraries
dnl
AC_DEFUN([CF_X_TOOLKIT],
[
AC_CHECK_LIB(X11,XOpenDisplay,
	[LIBS="-lX11 $LIBS"],,
	[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])
AC_CHECK_LIB(Xt, XtAppInitialize,
	[AC_DEFINE(HAVE_LIBXT) LIBS="-lXt $X_PRE_LIBS $LIBS"],
	AC_WARN(
[Unable to successfully link X Toolkit library (-lXt).
You will have to check and add the proper libraries by hand to Makefile.]),
	[$X_PRE_LIBS $LIBS $X_EXTRA_LIBS])

])dnl
