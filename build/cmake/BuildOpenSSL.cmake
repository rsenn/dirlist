project(openssl)
enable_language(C)

add_definitions(-DENGINESDIR="/usr/lib/engines-1.1")
add_definitions(-DL_ENDIAN)
add_definitions(-DNDEBUG)
add_definitions(-DOPENSSL_API_COMPAT=0x10100000L)
add_definitions(-DOPENSSLDIR="/etc/ssl")
add_definitions(-DOPENSSL_NO_DYNAMIC_ENGINE)
add_definitions(-DOPENSSL_THREADS)
add_definitions(-DOPENSSL_NO_IDEA=1 -DOPENSSL_NO_RC5=1)
add_definitions(-DOPENSSL_NO_MDC2=1 -DOPENSSL_NO_DES=1)

add_definitions(-DOPENSSL_CPUID_OBJ=1 -DOPENSSL_USE_NODELETE=1)


include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/crypto)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/crypto/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/crypto/modes)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/crypto/include)

link_libraries(dl)

exec_program(perl "${CMAKE_CURRENT_SOURCE_DIR}" ARGS util/mkbuildinf.pl OUTPUT_VARIABLE MKBUILDINF_H)

foreach(H crypto/include/internal/bn_conf.h crypto/include/internal/dso_conf.h include/openssl/opensslconf.h)

exec_program(perl "${CMAKE_CURRENT_SOURCE_DIR}" ARGS
  -I.
  -Mconfigdata
  util/dofile.pl
  -oMakefile 
  "${H}.in"
  OUTPUT_VARIABLE TEMPF)
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${H}" "${TEMPF}\n\n")
endforeach()


file(WRITE ${CMAKE_BINARY_DIR}/buildinf.h "${MKBUILDINF_H}\n\n")

file(GLOB LIBCRYPTO_SOURCES
  crypto/aes/*.c
  crypto/asn1/*.c
  crypto/async/arch/*.c
  crypto/async/*.c
  crypto/bf/*.c
  crypto/bio/*.c
  crypto/blake2/*.c
  crypto/bn/*.c
  crypto/buffer/*.c
  crypto/camellia/*.c
  crypto/cast/*.c
  crypto/chacha/*.c
  crypto/cmac/*.c
  crypto/cms/*.c
  crypto/comp/*.c
  crypto/conf/*.c
  crypto/ct/*.c
  #crypto/des/*.c
  crypto/dh/*.c
  crypto/dsa/*.c
  crypto/dso/*.c
  crypto/ec/curve25519.c crypto/ec/ec2_mult.c crypto/ec/ec2_oct.c crypto/ec/ec2_smpl.c crypto/ec/ec_ameth.c crypto/ec/ec_asn1.c crypto/ec/ec_check.c crypto/ec/ec_curve.c crypto/ec/ec_cvt.c crypto/ec/ecdh_kdf.c crypto/ec/ecdh_ossl.c crypto/ec/ecdsa_ossl.c crypto/ec/ecdsa_sign.c crypto/ec/ecdsa_vrf.c crypto/ec/ec_err.c crypto/ec/ec_key.c crypto/ec/ec_kmeth.c crypto/ec/eck_prn.c crypto/ec/ec_lib.c crypto/ec/ec_mult.c crypto/ec/ec_oct.c crypto/ec/ec_pmeth.c crypto/ec/ecp_mont.c crypto/ec/ecp_nist.c crypto/ec/ecp_nistp224.c crypto/ec/ecp_nistp256.c crypto/ec/ecp_nistp521.c crypto/ec/ecp_nistputil.c crypto/ec/ecp_oct.c crypto/ec/ec_print.c crypto/ec/ecp_smpl.c crypto/ec/ecx_meth.c 
  crypto/engine/*.c
  crypto/err/*.c
  crypto/evp/*.c
  crypto/hmac/*.c
  #crypto/idea/*.c
  crypto/kdf/*.c
  crypto/lhash/*.c
  crypto/md4/*.c
  crypto/md5/*.c
  #crypto/mdc2/*.c
  crypto/modes/*.c
  crypto/objects/*.c
  crypto/ocsp/*.c
  crypto/pem/*.c
  crypto/pkcs12/*.c
  crypto/pkcs7/*.c
  crypto/poly1305/*.c
  crypto/rand/*.c
  crypto/rc2/*.c
  crypto/rc4/*.c
  crypto/ripemd/*.c
  crypto/rsa/*.c
  crypto/seed/*.c
  crypto/sha/*.c
  crypto/srp/*.c
  crypto/stack/*.c
  crypto/ts/*.c
  crypto/txt_db/*.c
  crypto/whrlpool/*.c
  crypto/x509/*.c
  crypto/x509v3/pcy_cache.c crypto/x509v3/pcy_data.c crypto/x509v3/pcy_lib.c crypto/x509v3/pcy_map.c crypto/x509v3/pcy_node.c crypto/x509v3/pcy_tree.c crypto/x509v3/v3_addr.c crypto/x509v3/v3_akeya.c crypto/x509v3/v3_akey.c crypto/x509v3/v3_alt.c crypto/x509v3/v3_asid.c crypto/x509v3/v3_bcons.c crypto/x509v3/v3_bitst.c crypto/x509v3/v3_conf.c crypto/x509v3/v3_cpols.c crypto/x509v3/v3_crld.c crypto/x509v3/v3_enum.c crypto/x509v3/v3err.c crypto/x509v3/v3_extku.c crypto/x509v3/v3_genn.c crypto/x509v3/v3_ia5.c crypto/x509v3/v3_info.c crypto/x509v3/v3_int.c crypto/x509v3/v3_lib.c crypto/x509v3/v3_ncons.c crypto/x509v3/v3_pcia.c crypto/x509v3/v3_pci.c crypto/x509v3/v3_pcons.c crypto/x509v3/v3_pku.c crypto/x509v3/v3_pmaps.c crypto/x509v3/v3_prn.c crypto/x509v3/v3_purp.c crypto/x509v3/v3_skey.c crypto/x509v3/v3_sxnet.c crypto/x509v3/v3_tlsf.c crypto/x509v3/v3_utl.c
#  engines/afalg/*.c
 #  engines/*.c
  crypto/cpt_err.c crypto/cryptlib.c crypto/cversion.c crypto/ebcdic.c crypto/ex_data.c crypto/getenv.c crypto/init.c crypto/mem.c crypto/mem_clr.c crypto/mem_dbg.c crypto/mem_sec.c crypto/o_dir.c crypto/o_fips.c crypto/o_fopen.c crypto/o_init.c crypto/o_str.c crypto/o_time.c crypto/threads_none.c crypto/threads_pthread.c crypto/threads_win.c crypto/uid.c

 )

add_library(crypto ${LIBCRYPTO_SOURCES})


file(GLOB LIBSSL_SOURCES
  ssl/*.c
  ssl/record/*.c
  ssl/statem/*.c
)

add_library(ssl ${LIBSSL_SOURCES})
set_target_properties(ssl PROPERTIES LINKER_LANGUAGE C)
