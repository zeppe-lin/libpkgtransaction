# Maintaining

The public model, identity domains, manual pages, pkg-config metadata, and tests
form one release contract.

Before release:

1. run shared and static builds separately;
2. run the complete native test suite;
3. run ASan and UBSan builds;
4. compile every public header standalone;
5. inspect SONAME and dynamic dependency isolation;
6. verify exact source 4 / resolver 4 / state 4 provider generations and bounded pkg-config intervals;
7. verify the reviewed ELF ABI surface and x86-64 by-value layout contract;
8. build and execute installed shared and static pkg-config consumers;
9. verify build/check requirement edges target construction authority;
10. verify documentation, CI, and release metadata;
11. replay the exact release mbox from its declared base.

Do not reinterpret an existing identity domain. Introduce a new domain/version
when semantic material changes.
