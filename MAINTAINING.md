# Maintaining

The public model, identity domains, manual pages, pkg-config metadata, and tests
form one release contract.

Before release:

1. run shared and static builds separately;
2. run the complete native test suite;
3. run ASan and UBSan builds;
4. compile every public header standalone;
5. inspect SONAME and dynamic dependency isolation;
6. verify the direct `libpkgsource >= 2.0.0` metadata floor;
7. verify documentation and release metadata;
8. replay the exact release mbox from its declared base.

Do not reinterpret an existing identity domain. Introduce a new domain/version
when semantic material changes.
