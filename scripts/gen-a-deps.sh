#!/bin/bash

: ${BUILDDIR='$(BUILDDIR)'}
: ${EXEEXT='$(EXEEXT)'}
: ${OBJEXT=.o}
: ${LIBEXT=a}
: ${PATHSEP="/"}

: ${A_CMD='$(AR) rcs {OUT} {DEPS}'}

: ${COMPILE_CMD='$(CC) $(CFLAGS) $(CPPFLAGS) $(DEFS) -c -o {OUT} {DEPS}'}
: ${LINK_CMD='$(CCLD) $(CFLAGS) $(LDFLAGS) -o {OUT} {DEPS} $(LIBS) $(EXTRA_LIBS)'}



DEFVAR="DEFINES += HAVE_"
DEFVAR="DEFS += -DHAVE_"

TAB="	"
CR=$'\r'
LF=$'\n'
NL="$LF"
IFS="$LF$CR "
BS="\\"
FS="/"

pushv_unique() {
 v=$1 s IFS=${IFS%${IFS#?}};
 shift;
 for s in "$@"; do
  if eval "! isin \$s \${$v}"; then
    pushv "$v" "$s";
  else
    return 1;
  fi
done
}

pushv() {
  eval "shift;IFS=\" \$NL\$IFS\";$1=\"\${$1:+\$$1\${SEP-" "}}\$*\""
}

isin() {
 (needle="$1";
  while [ "$#" -gt 1 ]; do
    shift;
    test "$needle" = "$1" && exit 0;
  done;
  exit 1 )
}

match_expr() {  
  (EXPR="^(|\.[/\\\\]|.*[\\\\/])$1\$";  debug "match '$EXPR'"; shift; IFS="$NL"; echo "$*" |  ${GREP-grep} -E "$EXPR" );
}

indir() {
 (B=; : ${S=" "}; for ARG; do 
  case "$BUILDDIR" in
    */ | *")") B="${B:+$B$S}$BUILDDIR$ARG" ;; 
    *) B="${B:+$B$S}$BUILDDIR/$ARG" ;; 
  esac
done
echo "$B")
}


list() {
 (IFS=" $IFS"; set -- $*; IFS="$LF"; echo "$*"); }

 dirnames() {
   (IFS="$NL"; echo "$*" |sed "\\|[/\\\\]|! s|.*|.|; s|/[^/]*$||")
 }
 debug() {
  [ "$DEBUG" = true ] && echo "$@" 1>&2
}
implode()
{
  (if [ "${#1}" = 1 ]; then
    IFS=$1;
    shift;
    echo "$*";
  else
    sep=$1 out=;
    if shift && [ "$#" -gt 0 ]; then
      out=$1;
      while shift && [ "$#" -gt 0 ]; do
        out="$out$sep$1"
        shift
      done
      echo "$out"
    fi
  fi)
}

grep_e_expr() {
  (IFS="|$IFS";  echo "($@)")
}
#A_LIBS=$(echo "${@##*/}"|sort -u)
count() {
  echo $#
}
sort_args() {
  (IFS="$NL$IFS"; echo "$*" | sort -fu)

}
sort_var() {
  (VARS="$*"; IFS=\"\$NL \"; set --; for VAR in $VARS; do eval "set -- \"\$@\" \$$VAR"; done; echo "$*"|sort -fu)
}
get_objs() {
  [ $# -gt 0 ] && {
    strings "$@" |
    sed -n  "\\|^\([[:alnum:]_]\+\)\\.[co]| { s|^\([[:alnum:]_]\+\)\\.[oc].*|\1${OBJEXT}|; \\|lib/|! s|^|lib/|; p }"

  } ||
  echo
}
count() {
  echo $#
}
get_srcs() {
  (list $SOURCES | ${GREP-grep} -E "$X_SRC")
}

all_program_rules() {
  IFS="$NL "

  set -- $(echo  "$PROGRAMS"|  sed "s|^|${BUILDDIR}| ; s|\$|${EXEEXT}|")
  IFS=" $NL"
  O=
if [ "$GNU" = true ]; then
  pushv O "$@: ${BUILDDIR}%${EXEEXT}: ${BUILDDIR}%${OBJEXT}${LIBS:+ $(implode ' ' $LIBS)}"
   #[ -n "$A_LIBS"] && pushv O "\\${NL}$TAB"$(sort_args $A_LIBS)

   [ -n "$LINK_CMD" ] && O="$O$NL$TAB$(link_cmd)"
   output_cmd "$O"
else

   for PROG in $PROGRAMS; do 
     OUT=$(indir "$PROG${EXEEXT}")
     OBJ=$(indir "${PROG}$OBJEXT")
     DEPS="$OBJ${LIBS:+ $(implode ' ' $LIBS)}"
     output_cmd "$(pathfmt $OUT): $(pathfmt $DEPS)$NL$TAB$(link_cmd)$NL"
   done
     
fi
   


 }

output_cmd() {
  (IFS=" "; CMD="$*"; echo "${CMD}${NL%$LF}")
}
pathfmt() {
  (IFS=" "; echo "${*//$FS/$PATHSEP}")
}
a_cmd() {
  ([ -n "$DEPPARAM" ] && DEPS="$DEPPARAM" || DEPS=$(pathfmt $OBJS)
  CMD=${A_CMD//"{DEPS}"/"$DEPS"}
  [ -n "$OUTPARAM" ] && OUT="$OUTPARAM" || OUT=$(pathfmt "$LIB")
  CMD=${CMD//"{OUT}"/"$OUT"}
   echo "$CMD")
}

compile_cmd() {
  ([ -n "$SRCPARAM" ] && SRCS="$SRCPARAM" || SRCS=$(pathfmt $SRC)
  CMD=${COMPILE_CMD//"{DEPS}"/"$SRCS"}
  [ -n "$OUTPARAM" ] && OUT="$OUTPARAM" ||   {
  [ -n "$OBJ" ] && OUT=$(pathfmt "$OBJ") || OUT=$(pathfmt "$OUT")
 }
  CMD=${CMD//"{OUT}"/"$OUT"}
   echo "$CMD")
}
link_cmd() {
  ([ -n "$DEPPARAM" ] && DEPS="$DEPPARAM" || DEPS=$(pathfmt $DEPS)
  CMD=${LINK_CMD//"{DEPS}"/"$DEPS"}
  [ -n "$OUTPARAM" ] && OUT="$OUTPARAM" || OUT=$(pathfmt "$OUT")
  CMD=${CMD//"{OUT}"/"$OUT"}
   echo "$CMD")
}
# c_o_entry <$LIBEXT-file> <objects>
c_o_entry() {
  (A=$1; S="$2"
    SRCDIR=${S%/%*}
    shift 2
    O=
    pushv O "$A:"
    pushv O "${BUILDDIR}%$OBJEXT:" "$S"
    [ -n "$COMPILE_CMD" ] && O="$O$NL$TAB$(compile_cmd)"
    output_cmd "$NL$O")
}
# a_entry <$LIBEXT-file> <objects>
a_entry() {
  (LIB=$(indir "$1"); OUT=$LIB; O="$(pathfmt "$LIB"):"
    shift
    OBJS=$(indir "$@")
    O="$O $(pathfmt $OBJS)"
    [ -n "$A_CMD" ] && O="$O$NL$TAB$(a_cmd)"
    output_cmd "$NL$O")
}
dump() {
  eval "(IFS=\" \$NL\$IFS\"; debug \"\$1:\" \$$1 )"
}
gen_a_deps() {
  IFS="
  "
  [ $# -gt 0 ] || set -- build/*/*/*$LIBEXT
  A_LIBS="$*"
  set -- ALIGNED_ALLOC ALLOCA BSDSENDFILE C11_GENERICS CYGWIN_CONV_PATH DEVPOLL DYNSTACK EAGAIN_READWRITE EPOLL FNMATCHGETDELIM GETOPT GETOPT_LONG GLOB IO_FD_FLAGS IO_QUEUEFORREAD KQUEUE LIBBZ2 LIBLZMA LIBZ LOCALTIME_R LSTAT N2I PIPE2 POPEN POSIX_MEMALIGN PREAD ROUND SENDFILE SIGIO SOCKET_FASTOPEN UINT128 WORDEXP ZLIB STDINT_H \
  ALLOCA_H CONFIG_H ERRNO_H INTTYPES_H LINUX_LIMITS_H
  COND_DEFS="$*"
  set -- $(${FIND-find} . lib/ lib/*/ -follow -maxdepth 1 -name "*.c" | sed 's|^\./||'|sort -fu)

  SOURCES="$*"
  
  dump SOURCES
  SRCDIRS=
  for SRC ; do
    case "$SRC" in
      */*/*) SRCDIRS="${SRCDIRS:+$SRCDIRS$NL}${SRC%/*.c}" ;;
  ./*.c) SRCDIRS="${SRCDIRS:+$SRCDIRS$NL}." ;;
  *.c) SRCDIRS="${SRCDIRS:+$SRCDIRS$NL}." ;;
  esac
  done
  set -- $SRCDIRS
  IFS="
  "
  LIB_DIRS=$(list $A_LIBS|sed 's|/[^/]*$||' |sort -fu)

  LIB_NAMES=$(sort_args $A_LIBS | sed 's|.*/||' |sort -u)
  LIBS=$(sort_args $A_LIBS | sed "s|.*/||; s|\.[^./]*$|.$LIBEXT|" |sort -u| sed "s|^|${BUILDDIR}|" )


  dump LIBS
  dump LIB_NAMES
  dump A_LIBS
  dump PROGRAMS
  dump LIB_DIRS
  
  per_program_rules() {
    for P in $PROGRAMS; do
      (O=
        EXE=$(ls -d $(list $LIB_DIRS | sed "s|\$|/$P|") 2>/dev/null)
        dump EXE
        IFS=" ""
        "
        #SRCS=$(strings $EXE | sed ' s|\.[co]||p' -n|sort -fu)
        #OBJS=$(echo "$SRCS" | sed -n "/\.[co]/ { s|\\.[co]|| ; s|.*/||; s|^|${BUILDDIR}|; s|\$|$OBJEXT|; p; }"|sort -fu)
        OUT="$BUILDDIR$P"
        O=$(pathfmt $OUT)
        set -- $(list ${LIBS} | sed "s|.*/||; s|^|${BUILDDIR}| ; s|\$|$LIBEXT|" | sort -fu)

        O="$O $(pathfmt "$@")"
      [ "$LINK_CMD" ] && O="$O$NL$TAB$(link_cmd)"
      output_cmd "$O")
    done
  }

  SRCDIRS=`dirnames $SOURCES | sort -u`

  dump SRCDIRS
  per_srcdir_rules() {
    (SRCPARAM='$<'
    OUTPARAM='$@'

    for SRCDIR in $SRCDIRS; do
      ( F=$(${FIND-find} $SRCDIR -maxdepth 1 -name "*.c")
        IFS=" ""
        "
        OBJS=$(echo "$F" |  sed "s|\.c$|$OBJEXT|; s|^\.[/\\\\]|| ; s|.*[/\\\\]||; s|^|${BUILDDIR}|" )
        set -- $OBJS
        O=
        pushv O "$@: \\$NL$TAB"
        SRC=$SRCDIR/%.c
        SRC=${SRC#./}
        pushv O "${BUILDDIR}%$OBJEXT:${SRC:+ $(pathfmt $SRC)}"
        [ -n "$COMPILE_CMD" ] && O="$O$NL$TAB$(compile_cmd)"
        output_cmd "$O")
      done)
  }
  all_sources_rules() {
    for SRC in $SOURCES; do
    dump SRC
      OBJ=${SRC%.c}
      OBJ=${OBJ##*/}
      OBJ=$(indir "${OBJ}$OBJEXT")
      OUT=$OBJ
      output_cmd "$(pathfmt $OBJ): $(pathfmt $SRC)$NL$TAB$(compile_cmd)"
    done
  }

  a_lib_rules() {
    dump LIB_NAMES
    dump LIBS
    for A_LIB in $A_LIBS; do
      ( NAME=${A_LIB##*/}
        NAME=${NAME%.*}
        LIBEXPR="${NAME}\.(${LIBEXT}|lib|a)"
        FILES=$( match_expr "$LIBEXPR" $A_LIBS)
        dump FILES
        IFS=" ""
        "
        OBJS=$(IFS="$NL "; get_objs $FILES |sort -u)
        X=$(set -- $OBJS; IFS="|"; echo "($*)")
        dump OBJS
  #      dump X
  N_OBJ=$(count $OBJS)
  debug "$A_LIB:" $N_OBJ objects
  if [ "${N_OBJ:-0}" -gt 0 ] ; then
    X_SRC=$(echo "$X" |sed "s|(\(.*\))|\1|; s,[^ |]\+/,,g; s|\\$OBJEXT|.c|g")
          #dump X_SRC


          SRCS=$(ls -d -- $(match_expr "($X_SRC)" $SOURCES ) 2>/dev/null| sort -u)

          N_SRCS=$(count $SRCS)

          LIB=${A_LIB%.*}.$LIBEXT

          debug "$LIB:" "($N_SRCS sources)" 1>&2
          [ "$N_SRCS" -gt 0 ] &&
          dump SRCS

        #c_o_entry "%$OBJEXT" "$SRCDIR/%.c"
        [ "$N_OBJ" -ge 0  ] &&
        a_entry  "${LIB##*/}" $(list $SRCS| sed "s|lib/[^ /]*/||g ; s|lib/||g ; s|\\.c|$OBJEXT|")
      fi
      )
    done
  }
  IFS=" $NL"
  PROGRAMS=$(${GREP-grep} 'main\s*(.*)' *.c -l |xargs ${GREP-grep} -E '#include "(lib/|)' -l | sed 's|\.c$||')
  dump PROGRAMS

  O=
  SEP=$NL
  [ -n "$objext" ] && OBJEXT=$objext
  [ -n "$libext" ] && LIBEXT=$libext
  pushv O "CC = ${CC:-gcc}"
  if [  -n "$link" ]; then
   pushv O "LINK = ${link}"
   pushv O "CCLD = \$(LINK)"
  else
    pushv O "CCLD ?= ${CCLD:-\$(CC)}"
  fi
 if  [ -n "$TLIB" ]; then
   pushv O "LIB = ${TLIB}"
  A_CMD=${A_CMD//tlib/'$(LIB)'}
  fi
  [ -n "$AR" ] && pushv O "AR = ${AR:-ar}"
  pushv O "CFLAGS = ${CFLAGS:--Os}"
  pushv O "DEFS = ${DEFS}"
  pushv O "CPPFLAGS = ${CPPFLAGS}"
  [ -n "$exeext" ] && pushv O "EXEEXT = ${exeext}"
  #[ -n "$builddir" ] && pushv O "BUILDDIR = ${builddir%/}/"
  [ -n "$builddir" ] && {
    pushv O "BUILDDIR = ${builddir%/}/"
     BUILDDIR='$(BUILDDIR)'
}

  output_cond_defs() {
  SEP=$NL
  if [ "$COND_DEFS" ]; then
   var_get() {   eval "echo \"\${$1}\"";   }
   var_isset() {   eval "test \"\${$1+set}\" = set";   }
   for COND in $COND_DEFS; do
     if var_isset "$COND"; then
       pushv COND_VARS "$COND = 1"
     else
       pushv COND_VARS "#$COND ="
     fi
     pushv COND_IFDEFS "ifneq (\$($COND),)${NL}$DEFVAR${COND}=\"\$($COND)\"${NL}endif"
   done

  fi
  pushv O "$COND_VARS${NL}${NL}$COND_IFDEFS"
}
: output_cond_defs
  SEP=$NL$NL

  pushv O "all:"  $(pathfmt ${BUILDDIR%[/\\]}) $(pathfmt $(echo "$PROGRAMS" |sed "s|.*/|| ; s|.*|${BUILDDIR}&${EXEEXT}|"))
  pushv O "$(pathfmt ${BUILDDIR%/}):$NL$TAB@mkdir \"$(pathfmt ${BUILDDIR%/})\""
  output_cmd "$O"
  unset SEP

     all_program_rules
     [ "$GNU" = true ] && per_srcdir_rules ||
  all_sources_rules 
  a_lib_rules

} 
gen_bcc32_makefile() { 
  ( # A_CMD='LIB -out:$@ $^' 
    # A_CMD='(cd $(BUILDDIR); for OBJ in $(notdir $^); do echo tlib /u /A /C $(notdir $@) $$OBJ 1>&2;  tlib /u /A /C $(notdir $@) $$OBJ; done)'

    A_CMD='tlib @&&|
/p512 /a "{OUT}" {DEPS}
|'

:  ${BUILDDIR=build/bcc32/Debug/}

  OUTPARAM='$@' \
    PATHSEP="\\" \
    NL="$CR$LF" \
    LIBEXT=lib \
    OBJEXT=.obj \
    EXEEXT=.exe \
    CC="bcc32c" \
    CFLAGS="-O" \
    TLIB="tlib" \
    DEFS="-DHAVE_ERRNO_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_LINUX_LIMITS_H=1 -DHAVE_STDBOOL_H=1 -DHAVE_STDINT_H=1 -DHAVE_SYS_FCNTL_H=1" \
    gen_a_deps "$@" |tee ${OUT:-Makefile.bcc32})
}
case $1 in
  -bcc)  shift; gen_bcc32_makefile "$@" ;;
*) : ${GNU=true};  gen_a_deps "$@" ;;
esac

