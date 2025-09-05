ASN.1 library for C++
=====================

**Current progress:**

- implementing decoding of ASN.1 payloads
- representing subset of schemas required to decode context dependent objects

**Next:**

- implement encoding ASN.1 payloads
- BER / DER rules
- configurable nesting limit or iterative recursion
- find test vectors
- add test suite
- benchmark


**Future:**

- implement schema parser
- implement programming language type mappings

Usage
=====

```
asn1 <file>
```

Decodes an ASN.1 payload, and prints it out in human-readable form. `file` can be an X.509 certificate, for example.

Build and run
=============

(The provided test_payload.crt is a cert for letsencrypt.org))

```
$ g++ asn1.cpp -o asn1 && ./asn1 test_payload.crt
SEQUENCE(
SEQUENCE(
(constructed)
INTEGER 02
INTEGER 03d415318e2c571d2905fc3e0527689d0d09
SEQUENCE(
OID(2a.0348.01bb8d.01.01.0b.)
NULL
)
SEQUENCE(
SET{
SEQUENCE(
OID(55.04.06.)
PrintableString US
)
}
SET{
SEQUENCE(
OID(55.04.0a.)
PrintableString Let's Encrypt
)
}
SET{
SEQUENCE(
OID(55.04.03.)
PrintableString Let's Encrypt Authority X3
)
}
)
SEQUENCE(
UTCTime 190929163336Z
UTCTime 191228163336Z
)
SEQUENCE(
SET{
SEQUENCE(
OID(55.04.03.)
PrintableString letsencrypt.org
)
}
)
SEQUENCE(
SEQUENCE(
OID(2a.0348.01bb8d.01.01.01.)
NULL
)
BIT STRING 3082010a0282010100d0027597588872d78d4873aa668710d67f998a74e7e6db270d1186690230ddf68aa3cfd3fdbf281462eb417cefe8ad48d70aa50fc7426126e4da1a1faac7ccfba286ff37e2883d4cfb0778cd103a468639c474d35874906465220ca75f31bb9123bb85301449b2dfcc20f81e9f2cc0b2470b3efceec9ffd3d0ef3e425d78786542fae07e7197aac1cba925e8f54b595eb8b0612d07442b7974a08b4ce400992e5ecca1b6fbb2d572e4101ac1b5b2dddfdadd3ddb327c0438ff840774661f2f291c240d9bcfcd9ce8efa838a46b553d18667039d52e29f83e6b32b167af11dce926134eec55c2b02036696a3878f0182ab47d063c8d94f010d9090a948fdc271f0203010001
)
decoding failed: unknown type
```

References
==========

Recommendation ITU-T X.680 International Standard 8824-1 Information technology - Abstract Syntax Notation One (ASN.1): Specification of basic notation (Identical to ISO/IEC 8824-1)

Maybe:

RFC 5280: Internet X.509 Public Key Infrastructure Certificate and Certificate Revocation List (CRL) Profile
