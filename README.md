ASN.1 library for C++
=====================

**Current progress:**

- implementing decoding of ASN.1 payloads
- representing subset of schemas required to decode context dependent objects

**Next:**

- implement encoding ASN.1 payloads
- BER / DER rules
- find test vectors
- add test suite
- benchmark


**Future:**

- implement schema parser
- implement programming language type mappings

Build and run
=============

(decodes test_payload.crt: a cert for letsencrypt.org)

```
$ g++ asn1.cpp -o asn1 && ./asn1
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
decoding failed: unknown type
```

References
==========

Recommendation ITU-T X.680 International Standard 8824-1 Information technology - Abstract Syntax Notation One (ASN.1): Specification of basic notation (Identical to ISO/IEC 8824-1)

Maybe:

RFC 5280: Internet X.509 Public Key Infrastructure Certificate and Certificate Revocation List (CRL) Profile
