# Changelog

All notable changes to DeterministicESPAsyncWebServer are documented here.

## [Unreleased]

### Bug Fixes

- handle the bare 'native' core-engine env in the report merge + env selector ([`c69b1c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c69b1c9da94da26b1e2b8797bac7630db6f6aeee))
- split hot flag/handles from cold buffers so client builds gc-drop server/worker state ([`73ced5d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/73ced5df6365c1278799d7a1729d784696e17877))
- split the bump arena into its own owned symbol so --gc-sections can drop it ([`73cfe09`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/73cfe09f7eea210247cf6b0002ca7b444f140527))
- use nullptr instead of NULL (cpp:S4962) ([`0ea30fd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ea30fde86728a9fac5733a3ed6a9cfb8c87a166))

### CI / Build

- update test report + coverage [skip ci] ([`ff68ba7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ff68ba7f2e3059b2de2eded5a0d589c516d4cb4c))
- update CHANGELOG.md [skip ci] ([`2ab305d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ab305daa794d1ed434acf8d49fd967f400799be))
- update test report + coverage [skip ci] ([`497fb9f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/497fb9ff96f4cb5754cd6d916d1b064ec1af5168))
- update CHANGELOG.md [skip ci] ([`0223219`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/022321982b005400df08508b1131fca3bdc565bf))
- update CHANGELOG.md [skip ci] ([`7bafc2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7bafc2bdadd226e09ddecf2221c9297c8857b843))
- update CHANGELOG.md [skip ci] ([`a3095db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3095db96f546a848c9f6036810d0c7fba11a91f))
- update CHANGELOG.md [skip ci] ([`f655012`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f65501259b4e26a868bc9b3f5bf055366420f958))
- update CHANGELOG.md [skip ci] ([`a4cbd44`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4cbd44a24d42ac28085cb87a007f6e3129e991d))
- update CHANGELOG.md [skip ci] ([`1d26cb0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d26cb04559355afec4d16720dbc0543b7b20304))
- update test report + coverage [skip ci] ([`1e29495`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1e294955739e91015530bd613cfcf120bbf7964c))
- update CHANGELOG.md [skip ci] ([`9304848`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93048488add6f33ec2ff9001bd7178825adb3c9f))
- update CHANGELOG.md [skip ci] ([`e5c8eaa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e5c8eaa11a227abd7091cf2138ecfbc65d5bdee8))
- update test report + coverage [skip ci] ([`33064a6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33064a6eb614eff58a25b05f4d654d162b05bc40))
- update CHANGELOG.md [skip ci] ([`5d24fab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5d24fab6d9678769a7b0493e6717769527450e99))
- update test report + coverage [skip ci] ([`9926549`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9926549388e8839c458ac6fe1cf29233bdf836cf))
- update CHANGELOG.md [skip ci] ([`ab7f17f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ab7f17fc7eb3e0dac2601bb8513ef5274e820c2a))
- update test report + coverage [skip ci] ([`99d6078`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99d60782221e0d03ea6f8f041fb1b5ec0fe1d185))
- update CHANGELOG.md [skip ci] ([`1ecf3f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ecf3f48c846979fb90295ee179132bc6d8e64dd))
- update CHANGELOG.md [skip ci] ([`1657f3c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1657f3cf706cb38d897707b6edbe32772f93c22e))
- update CHANGELOG.md [skip ci] ([`b18efa7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b18efa79a20d9ec6106eedc40fc34e94159223c3))
- update CHANGELOG.md [skip ci] ([`4615f3c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4615f3c3045be2ef791525ade1a75f218357e537))
- update test report + coverage [skip ci] ([`830f622`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/830f6220fa55a33cddd5eb47f3ec679e778797c6))
- update CHANGELOG.md [skip ci] ([`458e990`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/458e9906943b8359358873b2e715a4261996c3c2))
- update CHANGELOG.md [skip ci] ([`3131ca7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3131ca71fa104f16f9f4bf8bb349eca097944cff))
- update CHANGELOG.md [skip ci] ([`e82e2dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e82e2dc4b4c61a7264a536efa8e7efbd04d16185))
- update test report + coverage [skip ci] ([`00ea482`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00ea48282792b5b0c480ae2214a9758bf973170e))
- update CHANGELOG.md [skip ci] ([`e160862`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e160862fdc7984f2e1ed0837f64e21660cafb421))
- update CHANGELOG.md [skip ci] ([`51bdf0a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51bdf0a5b885ef75226b3d628c0462cf5b9e58a5))
- update test report + coverage [skip ci] ([`757e51d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/757e51d5593f6d04fe41c6ec2feab766a1e8d237))
- update CHANGELOG.md [skip ci] ([`34bddf7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/34bddf793cb4219a4e1b184051a915e43b1699a8))
- update test report + coverage [skip ci] ([`f8b11a9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8b11a92f9c287df58dc92d93175c89d19b2ad5b))
- update CHANGELOG.md [skip ci] ([`1c98948`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c98948a52aa29507bffb1457cc8dd5c889c462a))
- update test report + coverage [skip ci] ([`a95c269`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a95c26990d86211f510627df45377f7d93dad5c6))
- update CHANGELOG.md [skip ci] ([`900cb77`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/900cb776d8f23f7b286bbcc90d12fcc93de27f2e))
- update test report + coverage [skip ci] ([`735d3dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/735d3dc08f2f7a8b0c992728864fd3c38fac7f11))
- update CHANGELOG.md [skip ci] ([`3de1dbd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3de1dbde70180372c78e9f516e1eca08e72aada1))
- update test report + coverage [skip ci] ([`a0b1a4b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0b1a4bf0273c9bd1477c335afe6a209015f1c56))
- update CHANGELOG.md [skip ci] ([`5470f91`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5470f91a8db1d0ece8c679b36e6f997720a48fdc))
- update test report + coverage [skip ci] ([`fa40ba9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa40ba98d43824c6bbb71027d95f5980de49eb03))
- update CHANGELOG.md [skip ci] ([`8e70ec9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e70ec943616fd00763f448c2607bf998af49c3b))
- update CHANGELOG.md [skip ci] ([`1ffb314`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ffb314d470ef1ecc6d7b04751bad288c9bf069f))
- update test report + coverage [skip ci] ([`04dcea5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04dcea5adfb58005c2b580e8b67dbfee2be819e0))
- update CHANGELOG.md [skip ci] ([`a83628d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a83628d2986b5633eb0b9cb5d32edd5fe8d1e5a2))
- run only the envs the diff affects; keep report + coverage as committed, incremental artifacts ([`33f83e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33f83e40cb642a41ce0901c8676be35680fe946a))
- update test report [skip ci] ([`0c0381c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0c0381ccbe8ec48b73f0475f8fee89ddf4db1c10))
- update CHANGELOG.md [skip ci] ([`7f556cc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7f556ccc3b246536139b0147f3f7ca2dcca4a4a4))
- update CHANGELOG.md [skip ci] ([`bf522fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf522fbbee230bb119d1533c80ddcfa072870132))
- update test report [skip ci] ([`174e40c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/174e40c9464b290eb56c972a3b529fcad784f02c))
- update CHANGELOG.md [skip ci] ([`6ac8484`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ac848410b8cf2cd2a1be72d524dbedb9fcc8bea))
- update CHANGELOG.md [skip ci] ([`dfdb915`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dfdb9152912a1790d2f934293eb6347c5d1353fa))
- update CHANGELOG.md [skip ci] ([`a685433`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a6854331bddb898fa08f0f5a3e19e28e85c300bc))
- update CHANGELOG.md [skip ci] ([`31163dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/31163dd62117bb46486a2995b4129f42832fc6d0))
- update test report [skip ci] ([`1696f67`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1696f67bda34d0788f58c3c855332f0e1adac7e8))
- update CHANGELOG.md [skip ci] ([`e59721a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e59721a84a86431dba82b26806fdf4eb743076c5))
- update CHANGELOG.md [skip ci] ([`859d407`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/859d40764d6ed65f0f96c7535fc44f782c7641aa))
- update CHANGELOG.md [skip ci] ([`36eab72`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36eab72df771b1ef7e51c8e89b0fdb22ce5c4009))
- update CHANGELOG.md [skip ci] ([`bfac83b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bfac83b424d41a000c17fb28501db35acce6d0e8))
- update test report [skip ci] ([`2477087`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24770872941e2f48e501553e539a276d017c4c17))
- update CHANGELOG.md [skip ci] ([`ef7ab3b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef7ab3b2b1f0253d01b066fbe477b53c0f5e230b))
- update CHANGELOG.md [skip ci] ([`697d22a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/697d22aa87ac8c819a62debbd7a3e6cbd839ecab))
- update CHANGELOG.md [skip ci] ([`4968377`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4968377351bb336e3daeb2bf7f5edf3b4abfb3b1))
- update test report [skip ci] ([`39b8008`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/39b8008c5773182e0f5226de0403e0344c1ee438))
- update CHANGELOG.md [skip ci] ([`ef42554`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef4255449372e6aadf31d6d20be87c4750f42ecf))
- update CHANGELOG.md [skip ci] ([`e5ee29c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e5ee29c2176282408b9cab62b6961ae3d8984952))
- update CHANGELOG.md [skip ci] ([`6490442`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64904426f2ebb99c5ca7a19b1d8d680d24be92de))
- update CHANGELOG.md [skip ci] ([`0871283`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0871283229fbf2e4466586a4933f6e762896e826))
- update CHANGELOG.md [skip ci] ([`913c5aa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/913c5aabbb9f430a1ffc9290d91e34613e13f6a0))
- update CHANGELOG.md [skip ci] ([`605de4c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/605de4c2ffb64b22ba07c4366ef4a12d57f3f5f6))
- update CHANGELOG.md [skip ci] ([`e6357b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e6357b4f0eb43314cfc87012438947ab4b39e213))
- update CHANGELOG.md [skip ci] ([`1a31092`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a310926a2beea3476c0859e9ccd2802ca68aa85))
- update test report [skip ci] ([`d1a29a6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1a29a6a9b7001b5b7c742cbd36c32e8e69fbca3))
- update CHANGELOG.md [skip ci] ([`0fea07d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0fea07d7fb8d7e93964eb12b7df28fd0e32caf4d))
- update test report [skip ci] ([`32335e3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/32335e3c71a83e383a09a4206972b77bb4bf29ad))
- update CHANGELOG.md [skip ci] ([`d44e78d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d44e78d8a320bb5dbafa58f44bcc525e20e1c293))
- update CHANGELOG.md [skip ci] ([`c7c50ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7c50ec9c5113f03eb19afcf19aa528d77153f2a))
- update CHANGELOG.md [skip ci] ([`03c8b00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/03c8b00545facd0e5cf9831feeaafb9cc4b1cbdc))
- update CHANGELOG.md [skip ci] ([`2aba84f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2aba84f0bcd6db175a989713e67c70d1aaa353f0))
- update CHANGELOG.md [skip ci] ([`bd8e0c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd8e0c85c8e4ea9175561a407e5a17d7a44c27a2))
- update CHANGELOG.md [skip ci] ([`96275ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96275ced37a8ad91cfad94bb1c2fe86d39564987))
- update CHANGELOG.md [skip ci] ([`c22e632`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c22e6321999be835f8bc875510eb0595fc5e8ff7))
- update CHANGELOG.md [skip ci] ([`062a7a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/062a7a297d3d9aa70845a2c28086664b1efff7e6))
- update CHANGELOG.md [skip ci] ([`250ba48`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/250ba485be5c9f409ae5d73ba7fd4e3435229341))
- update CHANGELOG.md [skip ci] ([`ae42734`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae42734ba8e5160d4af18424b1441ceda044a92b))
- update CHANGELOG.md [skip ci] ([`9da05dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9da05ddfdde2c1a710d76bd0f5774fbdb280ca37))
- update test report [skip ci] ([`f794e0e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f794e0e9bb900b01bac5aa339bf9c188d36144e5))
- update CHANGELOG.md [skip ci] ([`55df77a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/55df77a34d6855e5257556de47ce88879bab90d2))
- update CHANGELOG.md [skip ci] ([`a2b5a2f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2b5a2f06b7cc6a7bb2ff99e2a80a74a6c6d378f))
- update CHANGELOG.md [skip ci] ([`2d2785d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2d2785d249c3478d26efb18754db8ae8a311f5c8))
- update CHANGELOG.md [skip ci] ([`be0cad5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be0cad539e4fe13723bc6984c731810cf81350ce))
- update CHANGELOG.md [skip ci] ([`6e55a99`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6e55a99f2405c6fbb27e133f822a1d7ba36abc48))
- update CHANGELOG.md [skip ci] ([`9efd46d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9efd46dda2a44c8eb6f61a3fcab8bb32ef492f57))
- update CHANGELOG.md [skip ci] ([`d6b9d3d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d6b9d3dce6e0f95e93927e8edf25f1de83add239))
- update CHANGELOG.md [skip ci] ([`e394e71`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e394e7126cae9151b70068bf9680f995eba83554))
- update CHANGELOG.md [skip ci] ([`1955b11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1955b11b916b46dbf11dc6bd0f251744d73bb98d))
- update CHANGELOG.md [skip ci] ([`ebdefea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ebdefea9c69a94007118d2af4fd973502904bf53))
- update test report [skip ci] ([`33a6e6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33a6e6cffa1bb90bbfc0debd0c6213d733967b58))
- update CHANGELOG.md [skip ci] ([`7ba1ba2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ba1ba2fb85ea181ad96761b80ea09ec21bfb654))
- update test report [skip ci] ([`643443b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/643443b413d01dd44a7e73a2191009788ad8029d))
- update CHANGELOG.md [skip ci] ([`a4cb022`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4cb022c8a84730711becfb939b2cb3d0aaa7bf6))
- update CHANGELOG.md [skip ci] ([`0043319`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0043319bc522c72735beb5ec0531d31ea5d4d6cd))
- update test report [skip ci] ([`780effa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/780effa816064475cb37e8f613e4bb4a18540d83))
- update CHANGELOG.md [skip ci] ([`52d5308`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/52d5308f8a9a14a7ff870c8bd9aef1e89f3c7ea7))
- suppress S125 false positive on ServerConfig.h section headers ([`17045a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17045a0f25aaa3b37fbaa655ba0aeede21f88816))
- update CHANGELOG.md [skip ci] ([`004ecc7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/004ecc7127fb6de8fa8b107632705d3b6002fd72))
- suppress design-constraint-conflict rules + the tls13_msg S3519 false positive ([`430074d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/430074d244afc366bf35d7e4b36e20c23412412f))
- update CHANGELOG.md [skip ci] ([`c9c634e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c9c634e30eb3bdd9eb4c27be33d3414ae0d52d61))
- update test report [skip ci] ([`203df44`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/203df44891e939f910f7053d88c3f08c49fa3bbb))
- update CHANGELOG.md [skip ci] ([`fb93a14`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb93a14d4c95cc682bc0296f6425b7413815a869))
- update test report [skip ci] ([`15d06ff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/15d06ff8dc52a0f529275a54fd1d6481e9a2a7d5))
- update CHANGELOG.md [skip ci] ([`5202ffc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5202ffcaf68cde59f692fce4e3b23b640b4c939d))
- update CHANGELOG.md [skip ci] ([`4fc1386`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4fc13865b9b9f44a6481d37200e8302ffafa7070))
- update test report [skip ci] ([`7151c6f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7151c6f752c2c3bd2b21766663a842b3ff0880ea))
- update CHANGELOG.md [skip ci] ([`bdb3890`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bdb3890a79c6c54cc98c02f8d56cf58d44c85e12))
- update test report [skip ci] ([`2beb794`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2beb794d700fa419b738ef8e79d36286ced42c0e))
- update CHANGELOG.md [skip ci] ([`4fd5ebd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4fd5ebd0efb37eab5570765a38aa1a6d1a36d63d))
- update CHANGELOG.md [skip ci] ([`f66c7a8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f66c7a8f8a93e6c3217676e0a2dac038f29e737a))
- update test report [skip ci] ([`67ef56e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/67ef56e3539fc0b1ee255e8bd2cdc43d6a1d6b71))
- update CHANGELOG.md [skip ci] ([`cf83943`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf8394376a19d675e4adfcd96e22d7c69bee26bb))
- update test report [skip ci] ([`c56dd14`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c56dd14d65dada64ee1492994f2f6f537350dc46))
- update CHANGELOG.md [skip ci] ([`6878f31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6878f317c838346ce64285cc78b24afb0edb3e90))
- add owner-context guard - fail on any loose file-scope mutable outside an owned Ctx ([`26b9556`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/26b955658f7dc21db8da92f886dc423c5a7aa835))
- update CHANGELOG.md [skip ci] ([`8643401`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8643401d73b8676cd2cfb18766d4427fd030bb6d))
- update test report [skip ci] ([`21ccadf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/21ccadf7990324d060e1be1f11b0cc0805a67b5b))
- update CHANGELOG.md [skip ci] ([`6587460`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6587460c1921064fcfd1e7868079f36bfe545a93))
- update test report [skip ci] ([`719ed9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/719ed9c6e06e3d932b64808316b5ce5487528bbc))
- update CHANGELOG.md [skip ci] ([`62f89fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/62f89fb224b2f95f18e4c13839980fa5a38b156b))
- update CHANGELOG.md [skip ci] ([`e01c07e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e01c07e8f5c50499ffd37fe9a76f99acba6059d5))
- update test report [skip ci] ([`d2e2ff6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2e2ff6981567275ac36c7dbb43f5a8503c90a19))
- update CHANGELOG.md [skip ci] ([`7b9c910`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7b9c9101b9b17c9a28e2864e6e852c80aecb1450))
- update CHANGELOG.md [skip ci] ([`a9e0de7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a9e0de765aeff45f8360f65092925fb7dc116791))
- update test report [skip ci] ([`ee742c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee742c86e7878cdd6b47e4bc0e9044e627378032))
- update CHANGELOG.md [skip ci] ([`8f141bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f141bfafb3bf697e9f7cbc78447d2cd6bcf225c))
- update CHANGELOG.md [skip ci] ([`a38f164`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a38f1642b872d7048c3d0ed4725e5224c8dbb9a4))
- update CHANGELOG.md [skip ci] ([`51e64f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51e64f5f97107015e15c42ed9070374b90b215f2))
- update CHANGELOG.md [skip ci] ([`683b707`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/683b707552ea17b8fdeffcf726e03bb707a1d972))
- update test report [skip ci] ([`f1dc56c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f1dc56c3a8648910fb76608c935019dde123a7c8))
- update CHANGELOG.md [skip ci] ([`d772206`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d772206007f06d7dda7ee2f8cadbb5dbf8b1be65))
- update test report [skip ci] ([`ef7a964`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef7a964fe5241f2f714667e7be3c756c0864d4b8))
- update CHANGELOG.md [skip ci] ([`5dd9ed1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5dd9ed15304de5fc07ec8cd1867d5f33bc1f4db8))
- update test report [skip ci] ([`1c2d482`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c2d48227d17889be8b221970c4bef740dbddf31))
- update CHANGELOG.md [skip ci] ([`353ac0d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/353ac0ddcff667cc7a51a60b729e51186b69271f))
- update CHANGELOG.md [skip ci] ([`3e2a537`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3e2a5375e95041072e1c4f32bdc48983290e5b77))
- update CHANGELOG.md [skip ci] ([`612038b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/612038bab65872d8cf2cfb5b2a28aaf48463b85a))
- update CHANGELOG.md [skip ci] ([`6f8ed2a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6f8ed2a9cf5b11873fbb060cef8d85e046a8ca3f))
- update CHANGELOG.md [skip ci] ([`38bc476`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38bc47640f750e4fbd82a459f119e3835f0e0330))
- update CHANGELOG.md [skip ci] ([`95059e6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/95059e6f45f0c303465d12e41414e919e48452ad))
- update CHANGELOG.md [skip ci] ([`358c4f9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/358c4f9e36036fdb0995b59fe0a3d329387215b2))
- update test report [skip ci] ([`4642dc7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4642dc719553243dbe11a98575d306ca988d1cb2))
- update CHANGELOG.md [skip ci] ([`ee7d2e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee7d2e910f6ae43879e62e7efa62f0a026aaf287))
- update CHANGELOG.md [skip ci] ([`2b91e54`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2b91e545a8befb874d99ca206847b3c4fd0ea62f))
- bump actions/upload-artifact from 4 to 7 ([`08b79d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08b79d9a016bec41180b8ee647dc7c0d79ba509b))
- drop ccache wrap - incompatible with 3.x dynconfig toolchain ([`0727844`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/072784457837ae4a9fbb1d0a706a2d006a3239b3))
- update test report [skip ci] ([`2091108`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2091108404f1d150d07e82bd99672b8e191fb5d3))
- update CHANGELOG.md [skip ci] ([`d090e8f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d090e8f8dc0926dec3923dcc31ac1271cd1cb4de))
- fix ccache toolchain wrap breaking every example build ([`b47f5eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b47f5eb3014825298fdf2362bbd4d236ce657533))
- update CHANGELOG.md [skip ci] ([`7f9880c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7f9880ce04167de209baa1d720c0881d7c25fcac))
- update test report [skip ci] ([`fb6e066`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb6e066eb26d084c1fcf9341d22d13bfe065eff0))
- update CHANGELOG.md [skip ci] ([`b88207c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b88207cf26776afb94def4c907c22395b3f510ec))
- update CHANGELOG.md [skip ci] ([`936db80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/936db8094bc2e31c0189838416223729f6d676f7))
- update test report [skip ci] ([`0bf5086`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0bf50860279a27b84622437133b7ed641addb7d3))
- update CHANGELOG.md [skip ci] ([`39b61a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/39b61a27b98af1c2358d3d18d0d7a0f341cee9d9))

### Changes

- one assignment/declaration per line (Sonar S1121/S1659) ([`fcf0049`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fcf0049682e20f031815c46d0a32796244725437))
- annotate provably-dead guards in stomp + nats parsers (GCOVR_EXCL) ([`af7b6a6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/af7b6a6284c893e7c028bb9140d755fb07d5b618))
- clang-format client.cpp include-comment alignment ([`96a8e81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96a8e810d52e26f6385693b0285606a67c30bcce))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`4f76335`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f763357e418ec562c88ff60ffb62b796d705cd0))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`0e6f89a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e6f89ade5edd0603fdbd09e66a897a2a14001e2))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`05720da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/05720da99cfe2faa251c340409d2cc8969147773))
- add routing / forwarding / inspection section ([`9866406`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9866406adac7a527348d2eae81849efdae88bfa6))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`18bb2f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/18bb2f8b833df541bf3f76910f282794d4f47745))
- add CNC / machine-tool connectivity section ([`153e236`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/153e2367756d7fb91007e55afff959651006b9dd))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`eea69ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eea69cef1976fa3ace70f1de3cbfcfa4553740be))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`5781af1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5781af1880e77099054421956d6efcc97c4008fb))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`ba5b577`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba5b577ddc0c3610e655aa1dd0ff5229f27ec397))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`a130004`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a13000491c82dbcd77f164596f8eb9deeb4a3d96))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`c521b7a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c521b7a6c17e4ad6cb3c8db2f78b4600693d729d))
- update ESP32 build footprints [skip ci] ([`f4ecfa4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f4ecfa4c88912b60a9bfc197b2b2cf70ebd0778d))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`02f9b1b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/02f9b1bdbd937c52d2623bcfbb593d1e51955c47))
- capture the working-thread thought-stream (discussion #15) ([`7be47ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7be47ab40817b05b4bde9f412c829b36b3cc570d))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`bdf7953`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bdf79536afe62adc9d70c9ee1282c69bf1cfa19d))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`1271fc7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1271fc76cdc5eee713cf5675aec618333a17f89c))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`f22fcca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f22fccab0481270badeff38cedf7a30bb0a2c267))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`e1d9842`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e1d984218de31b1d962cb46707c4d0f8955215ad))
- update ESP32 build footprints [skip ci] ([`d182aac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d182aac84672bbfb0616f27774b58d260aaae493))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`dde7170`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dde717081dc332adeded69055ebfd6bb04ae03c8))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`5f0a3d5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5f0a3d5e738199295a137841a498bea77738933b))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`b4b61d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4b61d17624854312741cd9053993999def2ea21))
- update ESP32 build footprints [skip ci] ([`4117137`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/41171371d9bd78528db8cfccb7f99352d2d8d585))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`84554bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84554bb27a89962d4e230ecf75edf1790135cc2d))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`8c05f08`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8c05f08e891dd19971ee6ec8939ed9612ddf9670))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`47a6848`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/47a6848c9ce351e38746431fa4ab069e16bf54ea))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`27a049b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/27a049bf6bc16ae17a303dfb664f84c19231334e))
- consolidate test docs into a generated test/README.md ([`3536ecb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3536ecb5fcb98d8af75cff1a5a4782bd8cf97a61))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`6743496`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6743496ec32b32a2fc867b1c4627d6968de3d4f2))
- update ESP32 build footprints [skip ci] ([`bc74a96`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bc74a965a7e17b21c22c6d259a23d9fbf780ea47))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`27ff2c7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/27ff2c7c5ae8214b13a3b6a73cb14310a38d8b28))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`cb2e066`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cb2e06609d1217f7f4aed9a2615ab32a4ebbfe2d))
- update ESP32 build footprints [skip ci] ([`7be47c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7be47c8a34b43f27d84758625bccc219591dbb00))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`2a3c1ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2a3c1edcd85e78a39001a4fee58cd588bf5ea8a4))
- update ESP32 build footprints [skip ci] ([`efda745`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/efda7455d59e6c394b17c709d81e70848791869a))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`d71d147`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d71d14785d53707de694f7dd4b0e75855321ccac))
- update ESP32 build footprints [skip ci] ([`167f73e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/167f73ee85d64afd92986a2dd3ee09731bd2c5df))
- reformat AUDIT.md table columns (prettier) ([`e95a6cc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e95a6cce499f7f0d072292a3860a3812b4a6cc64))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`97978f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/97978f825f7ee1d1d470752f2e789a0fb615e526))
- update standards, audit, and root readme ([`f31b5b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f31b5b3885998ff52b35eb37ef685c9d07c6e903))
- update ESP32 build footprints [skip ci] ([`4a2b56e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a2b56eac4b84dc71296c9647b2c675f5ee81a60))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`a72bdfd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a72bdfdf84e756876867a5acc5040cff070a6a58))
- update ESP32 build footprints [skip ci] ([`cca4be2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cca4be2e79f5f0db57045455a40e5d3680d206d8))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`2af27ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2af27bafa1068ae773cbbe08744f3d09352946a0))
- update ESP32 build footprints [skip ci] ([`b335fdf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b335fdf8dba025be30a436a0a624eb5bead649d7))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`b569b18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b569b187055ed536f14db9fc53950126a5905cd5))
- update ESP32 build footprints [skip ci] ([`81d7a47`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/81d7a477a7d010e062e0b6c5eedc9f0d5f3ddbc4))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`aa2da24`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aa2da24d322fd15df782a6a75881df527507de64))
- update ESP32 build footprints [skip ci] ([`761c44c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/761c44c1905984b7bb132e6d2a0798fcc1b3d55c))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`3eeb643`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3eeb6438298bcb5a158a709c675f29efceb25cd3))
- update ESP32 build footprints [skip ci] ([`46e24ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46e24ee7bebe28a6893c6d2954d401116296ac36))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`38a90c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38a90c141c6bf5e76f34d85a01a1f5e89131b2cf))
- update ESP32 build footprints [skip ci] ([`30a0844`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30a08441a34324bad7fbd3ca81bca1fd8346e67a))
- update ESP32 build footprints [skip ci] ([`510261c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/510261c45da7d9e7d0cca91322ead8918e6fac75))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`34cf1b8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/34cf1b87cfb0b215bc24138934d141ff2efbaa1f))
- update ESP32 build footprints [skip ci] ([`f0787f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f0787f2933a5249fe466b020baf6d980578f812f))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`2383c57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2383c57d6786728a3c9a91c7981dbcdb12836f8c))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`3382909`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3382909edbc081cfb28ab65bee28e84e7e50dd63))
- update ESP32 build footprints [skip ci] ([`a1e44d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a1e44d013263cc557e2ec1a9202cbb8da6d16c30))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`e0d33cf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e0d33cf4156e1bbf93e28ae279ad608407a3b2a9))
- update ESP32 build footprints [skip ci] ([`1cc701b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1cc701b3925aff6483fe2f1a0445ce411bc074e7))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`83b3f31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/83b3f31876e4531ba9a15703b928a9c224b7dc5c))
- update ESP32 build footprints [skip ci] ([`beeb7f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/beeb7f4b852bb9bb33aea5c683e4eaf5d2f438d7))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`a6344b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a6344b96efa55d426d7f6d4e82e1206b8dc9f768))
- update ESP32 build footprints [skip ci] ([`80044f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/80044f83174e9c133e647aee7f9832532052ecf7))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`047abbf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/047abbf096a4cae6c841c645ebf7fd959bd1fa91))
- update ESP32 build footprints [skip ci] ([`04a90d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04a90d7047ef3626d21aec1a81da9eb842982188))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`9edd590`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9edd590373cf9ed1e3e7833d56ff824118f78b0e))
- update ESP32 build footprints [skip ci] ([`d113c88`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d113c888a59204e37882fa331b0cd7843bf01d23))
- update ESP32 build footprints [skip ci] ([`f12db9a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f12db9a726f0a6daa09ae95374a67b2c7475b7e5))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`6c18038`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c18038eb7254fc2b6aa3eaf893b38196ecbbbec))
- update ESP32 build footprints [skip ci] ([`9c174d3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9c174d36e7422f2f9ce1565ef065fa4701d50295))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`4f70b3b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f70b3bb16006e8bea44181948126103c7fe9aa9))
- update ESP32 build footprints [skip ci] ([`f354b01`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f354b015c67f22f4a0e261d3601a3abc31a1f84d))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`300d48c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/300d48cfdbe2474c17731ea3f5d9cf23c9f66a37))
- update ESP32 build footprints [skip ci] ([`9e05e0f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e05e0f9b932094df2be32aff83c7258cec9196e))
- update ESP32 build footprints [skip ci] ([`a066d63`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a066d633cd7b5f0bd38df19d2a10b12de055be3d))
- correct stale header comment - /.well-known/core IS served ([`f98b5a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f98b5a317b922d5bc3c857945839d29319f45038))
- remove 4 limitations that have since shipped ([`ec478a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ec478a74d7616ef606538f423b9e62b9f204bdf0))
- correct stale test count (600+ -> 2,300+) ([`1a427a5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a427a5700c4fa7215d1f4986e3c4c70686d6a48))
- fix two broken references caught by a link audit ([`f3ee602`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3ee60265ec4bb6853a41c45783cfdc45929912e))
- de-stale SSH, Utility Tools, RFC Compliance, API return code ([`7775247`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/77752472adfc389c5add094d0e8ea4b96301ed41))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`86fed6b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/86fed6b6c34cd660d91d8d6827fbfb861f413e0e))
- regenerate stale sections from source + restructure ([`34a6e55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/34a6e55dbdcb9131c0ee29aa6310bd5c6f1f9a6c))
- declutter Doxygen nav tree + drop sidebar highlight bar ([`eb2bd81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb2bd810b2669b30eb7b0a7fc1e847c76b2ed5af))
- backfill 54 feature entries missing from the grid + add coverage guard ([`48d3be7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/48d3be78935ef6a569dfd1a6be98943c553a10c4))
- close the multi-MCU portability item as a won't-do (per user request) ([`bcef60e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bcef60e0ec60ea08fc9854c37b1796ba604238df))
- fix broken SonarCloud badge (sonarcloud.yml -> test-report.yml) ([`450c617`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/450c617280017a813c1519d0945673061b09679b))

### Features

- emit the Date header from any enabled time source (NTP/GPS/RTC), not just NTP ([`0ebafd2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ebafd26cf4c1de8be2be7ad0ef746d213876964))

### Refactor

- drop dead ssh_dh_finish; cover KDF edge paths + slot guards (73->100%) ([`a62267f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a62267f05455dce6e0f3899cacd87eaaf35a68ec))
- move MPR121 touch/release thresholds to ServerConfig.h (overridable) ([`e00567e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e00567ed578785498c25b7c2e0332c3020ab3434))
- move ADS1115 + INA219 device tunables to ServerConfig.h (overridable) ([`349af2e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/349af2e23be54807c386e395065e8878c11bfe54))
- own the /stats + /metrics render buffers in StatsCtx/MetricsCtx ([`304ccd6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/304ccd6392af30aae5536d238475f275f0fa1170))
- extract the duplicated MSG-envelope preamble into r_msg_preamble ([`2115d33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2115d3342b2f503ffaa3cc5443f55cdbe3db05b0))
- hoist the bounded XML string-builder into a shared primitive (dedup) ([`cc88478`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cc88478d88062a2918c4e9c998c96f0c1157932c))
- hoist the AES S-box into a shared primitive (dedup) ([`be31581`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be31581db93542c9cd6bbd62ee401706cf735f60))
- pull remaining tunable defaults into ServerConfig.h under their flag guards ([`f6bfc3a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f6bfc3ac6b462ca5eb5fed4fa757f71f5242ba6f))
- dedup repeated literals; move OPC UA + provisioning string DEFAULTS to ServerConfig.h under their flag guards ([`199a58b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/199a58ba6b99df8cf6d47ef9f9d58108b8c07016))
- drop det_ prefix, transport->tcp/udp, DeterministicESPAsyncWebServer->dwserver, DetWebServerConfig->ServerConfig ([`3f8fcbd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f8fcbd068f84040fe7dd0343c2f0a52270f0c37))
- own the group14 Montgomery constants in one Group14Ctx (owner sweep) ([`df4e000`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/df4e000caac9ac92da47f82e08638bf78fc69683))
- own the capture sink in one PromiscCtx (owner sweep) ([`8a73043`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a730433b29078dcd3224b219948f3398759cf65))
- own send/instance/webdav state in SendCtx/InstanceCtx/DavBufCtx/DavPutCtx (owner sweep) ([`7ab3a2f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ab3a2f5c74ab4fc4ed934ffc71190b9f509e957))
- own the HTTP/2 connection pool in one H2ServerCtx (owner sweep) ([`be674cb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be674cbb9b5eb01e1aace6bda79752737713a7a3))
- own accept-throttle/ip-throttle/allowlist/worker-queues in four owned contexts (owner sweep) ([`fda5008`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fda50085e72b28c00b16a0a012f73f8fa53b3e0b))
- own UDP state in UdpCtx and the host capture seam in UdpCaptureCtx (owner sweep) ([`71700e7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71700e78b86ea0cf1a1409c74ccb4294ea181354))
- own the outbound client pool in one DetClientCtx (owner sweep) ([`616e294`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/616e294bc098321f745aa0df01e0fc5861f2367c))
- own observability in ObsCtx and tcpip-thread flag in TransportCtx (owner sweep) ([`00c45c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00c45c2017e1539cacb9ee066a812a25a0b0db73))
- own PSRAM buffers in QuicServerPoolCtx and control state in QuicServerCtx (owner sweep) ([`5ce32e3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ce32e39024767acbe350fe04126203bb6ea620c))
- own the conn table + command cb in one TelnetCtx (owner sweep) ([`b1d8dbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b1d8dbed9ae025a01a382e19ff257ace6a87552a))
- own the outbound frag size in one WsCtx (owner sweep) ([`de57c5d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/de57c5da3f75a735e0f9ace6428f5dcbfb6676b2))
- own task state in WorkerCtx and defer queues in DeferCtx (owner sweep) ([`879b1fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/879b1fab2dd60ffaae7dd92c73b417d8558fb0fa))
- own the ProtoHandler dispatch table in one SessionCtx (owner sweep) ([`7c667f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7c667f50f0c9508bb923b470b2d2445e8bba3882))
- own the per-worker bump arenas in one ScratchCtx; make arena base alignment explicit (owner sweep) ([`ebb1f1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ebb1f1f8fd332fdb21317982097a1f6bd29edf5e))
- own the streaming-body hooks in one HttpParserCtx (owner sweep) ([`132d786`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/132d7869c8a4086fd4e3f2546128211548c60a5d))
- own arena/server/conns/client-auth/csess in five owned TLS contexts (owner sweep) ([`268c145`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/268c1452993a6eeea2d5fffebd1480db43e83c8a))
- own the per-connection deflate table in one SshCompCtx (owner sweep) ([`ddfdf9d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ddfdf9dc2913dcd13dc47b3be26e58de7150616b))
- own KEX pref + ed25519 host key in one SshTransportCtx (owner sweep) ([`c02f0aa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c02f0aa1a5831a3c4addfa94f12101e673b27b6e))
- own the password/pubkey verifiers in one SshAuthCtx (owner sweep) ([`38889c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38889c6bec6506fc3448dbaeb0c487b20e46976d))
- own local/remote forward tables in SshFwdCtx/SshRFwdCtx (owner sweep) ([`9eeb02a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9eeb02a67d3cce90a09303b6ed99535db05557bf))
- own the emit callback in one SshServerCtx (owner sweep) ([`c2b6268`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c2b62688c513892d3dfb3cd243ef2b8cdf8b78f3))
- own the slot map + init/close flags in one SshConnCtx (owner sweep) ([`5c61eca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c61eca1094d763ca4f83e89404e5123ebf3233f))
- own the channel-layer callbacks in one SshChannelCtx (owner sweep) ([`be21c76`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be21c762b02ccf466636f420589399d82c93d730))
- own lane state in PqCtx + backend-specific PqQueueCtx/PqRingCtx (owner sweep) ([`23e0797`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23e0797ba65873a1285546dff6e3b664cdad4fe2))
- own server identity + handlers + channel state in one OpcuaCtx (owner sweep) ([`72c5a5f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72c5a5fa3b75dfc0757f9b4297693d4989e13003))
- own the response buffer + conn id in one HttpClientCtx (owner sweep) ([`93fa612`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93fa612ba204ef00a112e99d0b7a1541ac28a2d2))
- own the connection + reassembly state in one WsClientCtx (owner sweep) ([`5ccc7c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ccc7c86223c59264057a0be472cda13e667e8c1))
- own the broker connection state machine in one MqttCtx (owner sweep) ([`9e36112`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e36112e7f7dda085a1ed537bf2c37cefe7ddf09))
- own the server + AP IP in one ProvCtx (owner sweep) ([`9ccf29e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ccf29e31c5ceebad1bf87b428b1c9277f452b47))
- own the fs + dest + upload state in one UploadCtx (owner sweep) ([`bb9ecc1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb9ecc159028c77ddd10972c01f5478217893229))
- own the server + auth + upload flags in one OtaCtx (owner sweep) ([`48aa8bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/48aa8bb192552f36de6218257fe1249215ae4516))
- own the server + cb + ws state in one WebTerminalCtx (owner sweep) ([`18799ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/18799ab472c0735792482a2fb14936409844ed43))
- own the server handle in one PartitionRoutesCtx (owner sweep) ([`739e285`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/739e28525a8ee69bb304867bf75b970cff8e1910))
- own the server + pin table in one GpioRoutesCtx (owner sweep) ([`7cb1ad8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7cb1ad8b0d6bdff379e7944445484a7fccfcb5e7))
- own the UART stream + last report in one Ld2410Ctx (owner sweep) ([`46484f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46484f6da8f6a801c768760b325115bc4dcd5fe1))
- own the server + SSE/ws paths in one DashRoutesCtx (owner sweep) ([`253977c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/253977c61d73b55ca95a5880424eac4439cc130f))
- own the widget table + values + control cb in one DashboardCtx (owner sweep) ([`08f8bea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08f8bea5c0dcf1a52923bc28010f8ac865785dd8))
- own the client config + format scratch in one SyslogCtx (owner sweep) ([`212d78b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/212d78ba50a1cc655deb5799b54178f4cb6d097f))
- own the I2C address in one Fdc2214Ctx (owner sweep) ([`91cc557`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91cc5572e6f2c4ae2f506f93f2b56fee58ac91ff))
- own the I2C address in one Ldc1614Ctx (owner sweep) ([`ebd666e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ebd666eb878e0abbfba68c5c0a2afe98ccb0f6e9))
- own the I2C address in one Mpr121Ctx (owner sweep) ([`563eb6b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/563eb6b59d4a1d00b5fd023ce3a0b6d67d829415))
- own the I2C address in one Vl53l0xCtx (owner sweep) ([`e2bb034`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e2bb034e65c775b463ef93ea0f58df4a4b37a243))
- own the I2C address in one Sht3xCtx (owner sweep) ([`df5d881`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/df5d881c204549f262768fb3ac3c6f9f3e0c9b6c))
- own the I2C address + PWM freq in one Pca9685Ctx (owner sweep) ([`d1e3510`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1e35105d30d69432720cc19aec2e7162307a614))
- own the I2C address in one Ads1115Ctx (owner sweep) ([`7b09746`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7b0974610687579202b2bfa68c08e65f5c00d185))
- own the I2C address + current LSB in one Ina219Ctx (owner sweep) ([`8f102c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f102c024fbd27112f2e16d2e4abf06f596d3905))
- own the parser+executor state in one GqlCtx (owner sweep) ([`11174f9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/11174f9d4ea0db6b64bb0e5fcfc7c87a0ffd4a4c))
- own the USM engine/user/keys/stats/buffers in one SnmpV3Ctx (owner sweep) ([`818efd5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/818efd518a1e6d62d2d3640155f9b3683a4ea1a9))
- own MIB+community in SnmpAgentCtx, scratch in SnmpReqCtx (owner sweep) ([`08ade9e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08ade9e52967de1e0530739e405d2fb2d2cf7731))
- own dispatch+RAM in VfsCtx and the FS backend in FsCtx (owner sweep) ([`ecb16ac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ecb16acef1ed9f661a24f84554ba8ed2d4799e0b))
- own the trap request-id in one SnmpNotifyCtx (owner sweep) ([`93112d8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93112d86ee99fc8ec2287c7283c6955a12fc93c8))
- own the data model + write cb in one ModbusCtx (owner sweep) ([`f0a36dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f0a36dd60d8843bedae0b3b9da4b54612c6dab9b))
- own the host test-epoch seam in one NtpSvcCtx (owner sweep) ([`f7767ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f7767ba68d4b8f4d9f3c0bfbce26413ae9983aeb))
- own the exchange scratch buffers in one Oauth2Ctx (owner sweep) ([`883b8c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/883b8c57f5aa456df02a9d32245f88cfc611041d))
- own the collector endpoint in one UdpTelemetryCtx (owner sweep) ([`7e67bac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e67bace902894cb5fe3a31c473ec63769beeebf))
- own resolve addr + done/ok flags in one DnsResolverCtx (owner sweep) ([`0f988ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0f988edbdb6b7d758498e1043bd1516b08ddc234))
- own stratum + refid in one NtpServerCtx (owner sweep) ([`f846e1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f846e1d93a5c8507f6113724298585470dae6a7b))
- own NVS handle + host table in one ConfigStoreCtx (owner sweep) ([`5458d6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5458d6c3cd84d35488e39e9acc5c89337b5e9773))
- own the simulator channel table in one DmaCtx (owner sweep) ([`8f2aea0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f2aea0747e310fe44d22e67f64e99410f619479))
- own the port table + uplink/stats in one GatewayCtx (owner sweep) ([`9581a16`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9581a16d4e56c3e9f49bceabc82855179b9d4963))
- own the bucket table in one LockoutCtx (owner sweep) ([`861e348`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/861e3486795e19d7cd0837b5e39502b0853ba2c3))
- own the line ring + trap in one LogbufCtx (owner sweep) ([`ef4a738`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef4a738c5e9a6932596c7a5971f289d53a153b89))
- own the sink + running flag in one BusCaptureCtx (owner sweep) ([`0022b26`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0022b2622ee0acfa1d3082fe5b2b4e073b7e245d))
- own the breach callback in one GuardrailsCtx (owner sweep) ([`7042281`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7042281755e2f7629f833bf049f42c24f7cd62dd))
- own the source table + active in one TimeSourceCtx (owner sweep) ([`903bbea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/903bbea10c6c48d6601fc81624d4e0c9778622e0))
- own the driver registry in one SouthboundCtx (owner sweep) ([`d964553`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d964553a2318d65fc9593bc7c637455b58e75226))
- own the client destination/tags/ready in one StatsdCtx (owner sweep) ([`e5393a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e5393a3ea6cb3ac8c84e8d96f268de905bad8219))
- own the HMAC secret + nonce counter in one CsrfCtx (owner sweep) ([`1fe6157`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1fe6157b6a76cedb0bb6a4b5fa684b26f30744c6))
- own the record ring + cursors in one AuditCtx (owner sweep) ([`d9f3fb6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d9f3fb6542f148757edd7b5f106236e350645fa0))
- own peer registry + radio binding in one EspnowCtx (owner sweep) ([`9e852e7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e852e737e5c6fc8a229a44285b0a7bcbcda2b7f))
- own the forwarding plane in one ForwardCtx (owner sweep) ([`0ba257e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ba257e80b42a5cf233dd671af8708293832acfc))
- own the A-record table in one DnsSrvCtx (owner sweep) ([`904aa59`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/904aa5998a2c2afe5814b1e03e782f45605dec13))
- own state in one FailsafeCtx (owner-refactor sweep) ([`0d980b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d980b22723ee792a04c9defd29df1af4e2cfe92))
- own CoAP state in one feature-gated CoapCtx (pattern reference) ([`6092e6b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6092e6bede068068225f74ca850572b574b19976))
- unnest ternaries into if/else (cpp:S3358) ([`e826411`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e826411c45275728b98dfc2395cd7b094ba63169))

### Testing

- deep branch coverage for wisun (->100%) + webdav (->95%) ([`7c69e67`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7c69e6711250101e3faf43f1c5da55ac44aa8710))
- cover build/parse fail-closed guards across 10 more services ([`e5b7c61`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e5b7c616e7b9e079d13f6be97be82d50731f63e0))
- cover host stubs, build/parse guards + edges across 10 services ([`1f0bcce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f0bcce657106be9b10c6fd7edcec8bcfb0b9d82))
- cover host stubs, guards + clamps across 10 more services ([`9e93f63`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e93f6310f4db6692337e2c3b04dd0a9a8d7beb0))
- cover codec guards, host stubs + escape paths across 10 services ([`4bfc4ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4bfc4ad5c195fde5d1d975be62e6a188d515c433))
- cover host stubs, switch arms + fail-closed guards across 10 services ([`7280879`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72808799d0e71275dd83bd7a1061cfbfa51e7ee0))
- cover fail-closed guards in proxy_protocol, vfs, profibus ([`9ff0dbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ff0dbe755bc266f087e994b0428d3dbc61f1e83))
- cover host stubs + fail-closed guards across 7 services ([`586b147`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/586b14792fcb1a8c3daf591ab3c9623a7806d406))
- cover encode/parse guard sub-conditions + int/bulk/nil edges (branch 70->88%) ([`b4d02e2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4d02e29d37673a03352fbabe6a855915bf87f8d))
- cover non-object/bad-member find, int string/non-digit rejects, invalid \u (branch 76->80%) ([`47c781b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/47c781b91b6ee6f88e71110535ff2b7641c6a60b))
- cover send-family guards + 405/WS-400/WS-426 paths (dwserver batch) ([`941535d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/941535d0cfdfb1577b962abc866e586db98b8c72))
- cover route-registration variant table-full guards (dwserver 867-934) ([`e24dbf2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e24dbf218418189d4875206582928dd735070fee))
- cover DetWebServer::restart + stop teardown (dwserver 801-825) ([`bbb4a96`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bbb4a96a5785c5df98ab18238a8f0d9118ba475d))
- cover overflow / null / parser-edge branches (branch 59->81%) ([`1448399`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/144839951239ef5bad29ac5d626ce261b15062c2))
- cover the host TWAI stubs fail-closed (77->100%) ([`c37827d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c37827d5607b671154358106520fcbe593277376))
- cover form-field null guards + host load/clear stubs (72->95%) ([`04118db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04118dbdc6b72e05178c22fb29c4faa28cdd5923))
- cover raw_to_uv gain clamp + host I2C stubs (70->100%) ([`7690457`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76904574f82d46eb78cb24676b57fa12930c916f))
- cover null-guards + host sampler stubs (65->100%) ([`f7437eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f7437eb08f707714e54971906f8bbb95ab1d25bd))
- cover the host I2C stubs fail-closed (58->100%) ([`0a8806b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a8806bef9757f30a8fa66555babbc7a3426c7ba))
- cover the host NTP seam - accessors + http_date guards + gmtime_r-fail (52.6->100%) ([`09f5c83`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/09f5c83fb37729379c7044181a10f3c1a5cb676b))
- cover worker_set_self + host lifecycle stubs + inline defer (50->100%) ([`a3e3b41`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3e3b4112b9ccb8de80571fc76d6a46ceca4a550))
- cover the host platform-hook stubs (50->100%) ([`7e6e63c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e6e63caac815f7cbe6ce9d9d37c711e6b23c7cb))
- cover ATT/GATT codec build + parse guards (83->100%) ([`3ad6787`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3ad6787ce04ab382e127bba84306b9b8f4d95466))
- cover PSID/WSMP/1609.2 codec guards (71->100%) ([`8303915`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/83039157efa13c71f4ecc34a3d1c2ff55baee325))
- cover ClientHello parse guards + malformed exts + builder caps ([`9f50a04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f50a04b62035559f501c1fafc6d6e5b25fa22bf))
- cover ssh_channel guard/error branches (91->100%) ([`71ea672`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71ea67206e5ddd88906e529cb0bf31afbd72c20a))
- cover ssh_server dispatcher guard/error branches (73->99%) ([`4630d65`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4630d653870b04e7ff71a1273e4f923375dfc3fe))
- cover parse guards + NOTIMP + table/begin edges; 83->100% ([`bd3292a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd3292aa33f1cd6f2dfdb6eb049b597954bcc659))
- injectable host UDP mock; cover CoAP Observe (76->99.5%) ([`dd44e43`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dd44e4334476663bfcaa68d8c717efb78819b28e))
- cover quic_conn send/crypto/stream edges ([`ac6504e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac6504e45893a4e72598ae6354580d602384055c))
- cover quic_conn recv-path guards (87->90%) ([`b69be4b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b69be4b8199a65a1d4972736924f585fee6ab3c0))
- cover the response-buffer capacity-stop path ([`031107e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/031107e1a62faccd45be6e9927949c64816f50bd))
- finish h2_conn.cpp to 100%; annotate dead respond guards ([`992da24`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/992da24360a286cba8dd5e026eb2ee42262fde31))
- cover the HTTP/2 connection frame handlers (57->94%) ([`b02e032`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b02e0328e076449e4d44fbb15bcc9170f60c3f1e))
- cover listen() + begin() server bring-up paths ([`b4e7ba6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4e7ba60ad960b4f44283f8d8ae7ce977bc3a4ae))
- cover status_text reason phrases + method_name Allow tokens ([`89ef8c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89ef8c6755fa329c34948fa2b4d7ea8c242ef6d8))
- cover Perl escape classes + char-class backslash escapes ([`e3a569a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e3a569ada56f65686e76fbf97594da89209c7ddf))
- cover WebSocket + SSE send API; isolate ws/sse pools in setUp ([`113b046`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/113b046e96eaccde1ab236b3992bdbbe024b0a42))
- cover WebDAV streaming-PUT sink; annotate dead DAV guards ([`e89afd3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e89afd36f5f86c25dcb6f24b27256891e24f1ab7))
- cover BER long-form length + pdu/frame guards (80->100%) ([`943da2e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/943da2e633e6f91e766e3bc6c6733e729531024a))
- cover final guard/VIFE branches (->100%) ([`a9e71a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a9e71a74d791ba0e1eb47149827be991d75f3eab))
- cover Fast Packet build/feed rejects; EXCL dead decode (92->100%) ([`f853684`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8536840bf16b08e08e030850b3f111707b91506))
- cover build cap/null guards + rdm_parse rejects (92->100%) ([`24c8522`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24c85225fa68388d59ab880eb06b76ec55977d28))
- cover build/checksum-hex/field-helper rejects (90->100%) ([`c4a72f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4a72f7b3827326d12db9beabce6c698e3db0471))
- cover builder/parser/CRC reject + skip branches (87->100%) ([`e4b7da2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e4b7da265459ae7ba83ebb95aab9ffb20388c867))
- cover writer/reader error + float + 16-bit-id paths (86->100%) ([`141af00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/141af002b1135c2ef44d34fa21a03e8e3894cf69))
- cover build/TP error paths; EXCL 5 dead encode guards (88->100%) ([`7dbe6b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7dbe6b62d8e00ea390727110ac0f680bec07718d))
- cover id/frag error paths; EXCL 2 dead append guards (84->100%) ([`8b777a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b777a21bc83063431cd40f3b2dc4332126d0b3b))
- cover DISCONNECT teardown + overlong-banner reject (91->95%) ([`c95be3a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c95be3a6ac135dc0a3d9f0aaa772ab4a2fff7d8f))
- cover channel send/close/open + accept/poll paths (64->91%) ([`42e4097`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/42e409718aacd8a4d78213725c746d814900b01d))
- cover L5 accessor + send/poll/rx guards; drop stale exclusions ([`c57598d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c57598ddb34a6ba345db2f864c79c79064fcde99))
- cover builder/parser guards (->100%) ([`828be72`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/828be7228a36c70277037768bbb0cf7a862656d3))
- cover reader/writer + trailer guards (->100%) ([`d1e8ebf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1e8ebfc6e9c5b3b4fd04b87725d1d2a953779e5))
- builder guards + parser errors; EXCL dead dnp3/cip guards ([`446830d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/446830d846cb8901fb204e44d5675d543639e8ec))
- cover parser edges + builder guards (->100%) ([`3f113aa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f113aa1e933dc8a72ef54d898f6305599ef587f))
- cover hex-decode + all reject guards (->100%) ([`1512fcd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1512fcdd14a7c57471def0d6e0949310a83d120e))
- cover remaining reject guards (->100%) ([`a7b5eca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a7b5eca160ce227dd3faa7081ae38a5fa342b48d))
- cover all builder/parser reject guards (88.8%->100%) ([`c290493`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c29049326fde9d16f16412f003c5eff6f0cfcfee))
- cover every dialogue error/overflow branch (78.8%->100%) ([`5314982`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/53149823767e10e01d88a86a4600173e8e8f1744))
- cover snmp_v3 USM edge rejects; annotate provably-dead guards ([`8aa9a85`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8aa9a859d2df89a78cd2d6f3ccfbd9cd05e3812e))
- cover BER long-form len + all parse fail-paths (64.7%->91.4%) ([`6c3d8c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c3d8c56281730585400801b1c6b6a79aa2a15eb))
- cover the dynamic table + error/overflow paths (69% -> 99.5%) ([`f5ce15e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f5ce15e736e059234c650310a5263ca6aa325d99))
- cover the decoder + error paths to 100% line coverage ([`a2b84fc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2b84fce12bfdf0075af76c9173c8669b7d9608b))
- per-run throwaway SSH keys + a fixed baseline set, a host-key generator, and a provisioning example ([`0c36935`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0c3693547db01d2baccefca217a9e16b2a5c57e5))
- data-driven external KAT env + fix Ed25519 signature malleability ([`804b66e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/804b66eb349af9b434e58ea03285d8f2429b8f12))
- homogenize output to 120 cols, right-justify the status ([`bf1a6d6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf1a6d656c5998fcdad4891ce72bb859b8632f8d))

## [5.62.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.62.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`ed72866`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed7286639fd3af11ba3b1843ee44fba078dccf5b))

### Changes

- Bump version: 5.61.0 → 5.62.0 ([`bfccf29`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bfccf2919055aba273cac11baf68855bad7c4bdc))

### Documentation

- per-standard conformance audit (docs/AUDIT.md) ([`b9d1548`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b9d154827de7ed9694716b284cbfb20a95da89c2))

</details>

## [5.61.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.61.0 - 2026-07-06</b></summary>

### CI / Build

- update test report [skip ci] ([`9631e16`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9631e1602eeebf614078e33c2e214e26470aa40b))
- update CHANGELOG.md [skip ci] ([`eb5c740`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb5c740630b34593eb9238d117daafb056c69808))

### Changes

- Bump version: 5.60.4 → 5.61.0 ([`fa8306e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa8306e11457d0593df8b39f0dded7b847a04605))

### Documentation

- Sphinx + Breathe bridge with the squirty (green-screen) theme ([`86ab1c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/86ab1c1daf30bc2849aedefd83d1d51c6358b6b3))

</details>

## [5.60.4] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.60.4 - 2026-07-06</b></summary>

### Bug Fixes

- express tlv copy length as n-k + NOSONAR the S3519 false positive ([`39676f0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/39676f0790487d82f6d52ebecd73bbdc2eea9ee7))

### CI / Build

- update test report [skip ci] ([`7661989`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7661989e3135f60ffed95d5cdad4c8963c8b2cdc))
- update CHANGELOG.md [skip ci] ([`6a76722`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a7672274ab6b97bb54ffd21309835282f518ab8))

### Changes

- Bump version: 5.60.3 → 5.60.4 ([`90f1fcd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90f1fcd8fafcbd52e4f50a1f658369783c360b48))

</details>

## [5.60.3] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.60.3 - 2026-07-06</b></summary>

### Bug Fixes

- clamp tlv value copy to remaining room (clears S3519 at source) ([`5d6ac87`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5d6ac87bcd72fc8aece5337c9b332e910058ec9c))

### CI / Build

- update test report [skip ci] ([`7abc3a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7abc3a051e11d1077f6649ca87827cc581250e32))
- update CHANGELOG.md [skip ci] ([`bc63f4a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bc63f4afb448fe6dede9b5553d0503c78159a8a4))

### Changes

- Bump version: 5.60.2 → 5.60.3 ([`9004f76`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9004f76c71c2906881d52da037a6990c23c8a535))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`048575a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/048575a8b4d8d2bfbc566d17e09d7aaa5e3da5a5))

</details>

## [5.60.2] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.60.2 - 2026-07-06</b></summary>

### Bug Fixes

- clear remaining mms bug + raise new-code coverage over 80% ([`d3f5440`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3f544078938de4dd78949efcb08f8cad1e44b76))

### CI / Build

- update test report [skip ci] ([`8e39351`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e39351c71f7ef5ec0a3d36270ccfb25a123e402))
- update CHANGELOG.md [skip ci] ([`3c74008`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c740089aba795e87e508e530a9e1aa35d8e975a))

### Changes

- Bump version: 5.60.1 → 5.60.2 ([`ffcc2d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ffcc2d18a29721043a26543ef64400bbbff2a9ff))

### Documentation

- update ESP32 build footprints [skip ci] ([`d479b77`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d479b7762cf22edd0bfb2c11704fae526f3ad294))

</details>

## [5.60.1] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.60.1 - 2026-07-06</b></summary>

### Bug Fixes

- resolve SonarCloud new-code reliability + coverage gate ([`f62d9bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f62d9bb109842b8dee793924e1aef83ac3209e3d))

### CI / Build

- update CHANGELOG.md [skip ci] ([`85169c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/85169c617ecd8215cd45f1ce40b6afaac7e03f6a))
- update CHANGELOG.md [skip ci] ([`ff15f05`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ff15f05e0c0570372f804e7c7b1310972c65394d))

### Changes

- Bump version: 5.60.0 → 5.60.1 ([`d5f7f0c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d5f7f0ccf3caa580cded61c29a087ac4c0b05aaf))

### Documentation

- mark Refresh build footprints [x] (CI auto-maintains footprints.json/FOOTPRINTS.md) ([`ede68ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ede68ab855d5f118d263810aed03231dccc142c8))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`121bc9d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/121bc9dcd7f721c9ce96a3f05dfaa59b84ba88e6))
- update ESP32 build footprints [skip ci] ([`1f746ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f746ba52f8ef1f3599c780af4cf45d604c57f0c))

</details>

## [5.60.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.60.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`3ba4036`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3ba40367e60e7547cd845f45d42569846bfc81ad))

### Changes

- Bump version: 5.59.0 → 5.60.0 ([`1817162`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1817162c199df723f0bcc6817d1ab86aa81ce1d0))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`807182e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/807182e9e90aef28307fd239d07e248f0b13c6db))

### Features

- server-initiated session rekeying by volume/time (RFC 4253 sec 9) ([`8fa7ba9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8fa7ba9112d0178f0e1f34ea0a909b1dfeb6ce20))

</details>

## [5.59.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.59.0 - 2026-07-06</b></summary>

### CI / Build

- update test report [skip ci] ([`65e3ebd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/65e3ebd9bebb23d507716e9813b8573221077e0f))
- update CHANGELOG.md [skip ci] ([`d66ced0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d66ced0a0324e06a3818b44fae6384db02ac037a))

### Changes

- Bump version: 5.58.0 → 5.59.0 ([`88ba48d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/88ba48d818b370486cb3cfff6f64ab2dc3f09a52))

### Documentation

- update ESP32 build footprints [skip ci] ([`b4e5d6d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4e5d6d365bc865e9364d84b12bed38584ebc988))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`742f661`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/742f661dd3c38bac4cb74bd7023185eaf07398b4))

### Features

- per-direction NEWKEYS cipher activation (RFC 4253 sec 7.3) ([`623ae31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/623ae317e52d4dd3b52e06d18084610621dddd24))

</details>

## [5.58.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.58.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`1c740be`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c740be21b338a15f09e8ac4b143ea6a43b76e4c))
- update CHANGELOG.md [skip ci] ([`489d02a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/489d02a0dfe73f22938094c7bfc500d5cd7ecda4))
- update CHANGELOG.md [skip ci] ([`d1564ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1564ab5ecb781824ee520ea9750607be6003c92))

### Changes

- Bump version: 5.57.0 → 5.58.0 ([`d4ff947`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d4ff9470a98c4e0b952d12cbf081638f19c9fe0e))

### Documentation

- update ESP32 build footprints [skip ci] ([`cd89cc4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cd89cc4d3ded0e83e5345d214a8672a17863021d))
- reconcile post-v5 southbound backlog to [~] (per-module codecs shipped) ([`37f751a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/37f751a8d8385ab12df36d34e5decf8d130289f4))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`5152758`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51527588bd8f4e3d0d0d4757024a3ad19c76b2d6))
- reconcile build-flag tree + build configurator to [~] ([`3c8ac3c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c8ac3c4ec2a826a540aee403db93715d0d5c455))
- reconcile Fieldbuses to [~] (CANopen/PROFIBUS/DeviceNet/Modbus shipped) ([`7c34625`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7c34625de0d539efe27bcf5cf54767322fed73cb))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`50d617e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50d617e65fa97c3b327fcf1f94f31d640fc05755))
- update ESP32 build footprints [skip ci] ([`d6c09ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d6c09cef2c20a05a61b4e3451439b68257c18b05))

### Features

- Wi-SUN FAN border-router connector (CoAP client + FAN node registry) ([`f1adf64`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f1adf649a7ea32e4d253715f8a50fdb687b4fba1))

</details>

## [5.57.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.57.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`175f834`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/175f83423df05f8e05415be0ddbe57c2ef5e00fa))

### Changes

- Bump version: 5.56.0 → 5.57.0 ([`14b388a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14b388a00f85c62e1a86b91ce6af3bd1d0fa3f06))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`4db2724`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4db27240aad5fde38a8959534d9cb02219b4ec1c))

### Features

- TLS version negotiation + pinned cipher-suite policy ([`62b2619`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/62b2619fe2182d1993f2751a7269c12847563520))

</details>

## [5.56.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.56.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`0cd620f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0cd620fdc880a6d0363139879b8acaaa787d64a0))

### Changes

- Bump version: 5.55.0 → 5.56.0 ([`7bf8014`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7bf80140fc1ce29b0edfb58d9ae36e3e08083fe6))

### Documentation

- update ESP32 build footprints [skip ci] ([`e44e4a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e44e4a13167e01bf629ccc3259f3a0231ff4d13f))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`5b02ad1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b02ad12a5790801625a0556a39fc5c9447e3719))

### Features

- Bluetooth ATT protocol codec + GATT characteristic bridge ([`2124e08`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2124e0889a901b6d62bd0071d2ead8b6b8fe2130))

</details>

## [5.55.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.55.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`c95199d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c95199daba0b6dc349b37b78b85a12353bcdeb95))

### Changes

- Bump version: 5.54.0 → 5.55.0 ([`de1a5d6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/de1a5d6a06273ddaeb4d425d22e5b4b11200c72d))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`0a95b2d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a95b2d14da065e698d82ddf7ba6d3064d6d5da7))
- update ESP32 build footprints [skip ci] ([`9b67d3e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b67d3e6c4b9b598e4a7f4b50283b55d902266e7))

### Features

- receive-only radio channel sniffer to pcap (802.15.4 TAP) ([`3a2d625`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a2d625b77a0a2c3bbe95341b1e4a966373c256d))

</details>

## [5.54.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.54.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`daa8364`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/daa83647499ea00b01cf401eeaa822b11aa799cb))

### Changes

- Bump version: 5.53.0 → 5.54.0 ([`745a1d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/745a1d17c3008f445debbf18366986c132f47abd))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`82fab33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82fab33bc59e7f222f3ef5919ae3cd9351fbb03c))
- update ESP32 build footprints [skip ci] ([`752e9ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/752e9ef263f1ab8e39a47fc1dc558c06944256b0))

### Features

- VL53L0X optical time-of-flight ranging driver ([`46c2e4b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46c2e4ba9ec70b038db8be389643cd932daaecfa))

</details>

## [5.53.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.53.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`75ebb07`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75ebb072bc0f95850ab5f1b1407daac8ccff2e3a))

### Changes

- Bump version: 5.52.0 → 5.53.0 ([`8727dfa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8727dfa87cbe3ee7481fcb72af6dcae183b7bec6))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`4fddb5b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4fddb5bc9fc5cc092785565c0399e2e627152af6))

### Features

- LDC1614 inductance-to-digital field sensor ([`f3ec761`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3ec76126b423caee04c546eb07e808a2a7cef00))

</details>

## [5.52.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.52.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`1bf4f1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1bf4f1d69f7b25455cb91424a98c5a140abf38c6))

### Changes

- Bump version: 5.51.0 → 5.52.0 ([`562811f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/562811f27c924b5dbd9e6665104c4806f14b0fec))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`1e426c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1e426c58997edea51800a8ed7859fb7ef7bad06d))
- update ESP32 build footprints [skip ci] ([`d105ec9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d105ec927372f3cf45a701379c90f9f45db1652b))

### Features

- FDC2114/2214 capacitance-to-digital field sensor ([`eb59012`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb59012ba6d6f324d72236ebd53196bab9824293))

</details>

## [5.51.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.51.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`91a71af`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91a71afdc971a907217b0344a41beffdb5282683))

### Changes

- Bump version: 5.50.0 → 5.51.0 ([`345dccd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/345dccd872a47ae85dddca220c03dad797496eda))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`e4cd708`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e4cd70806fce49696364d2d1beb1f46155774f40))

### Features

- CC1101 sub-GHz radio driver (services/cc1101) ([`07d3ee2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/07d3ee25f097a68482f4683ccfb128b80d61ed75))

</details>

## [5.50.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.50.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`44c1373`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/44c1373c3b222d472fd2dd1b8b5a11354dcdf5a4))

### Changes

- Bump version: 5.49.0 → 5.50.0 ([`22c79de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/22c79de4f27087c7d51a177f0ee0e25627d32126))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`d5cc2c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d5cc2c918008c21bcf9da0f7f40bd58155baa323))

### Features

- multi-interface egress selection + escalation/failover ([`d9002b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d9002b3b87c434f4438b1fa3fc94709f78470737))

</details>

## [5.49.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.49.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`39f42b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/39f42b9da8b94559c1e8408d6235906728d8d114))

### Changes

- Bump version: 5.48.0 → 5.49.0 ([`03c1af6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/03c1af6b5796350c6c6f04217aaa0042f76b5628))

### Documentation

- update ESP32 build footprints [skip ci] ([`cf7729d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf7729dc55c91241914697a702a49318da3189e2))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`3a9863f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a9863f8d478c77c3f792bc7d8a1dd076d2a1dd9))

### Features

- 802.11 frame decode + traffic tally + roaming decision ([`f48fc38`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f48fc383c5a27a013a7324ec9148010c6e294330))

</details>

## [5.48.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.48.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`a00f325`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a00f3255845aba3a5e851b933a329635fa79d100))

### Changes

- Bump version: 5.47.0 → 5.48.0 ([`b49b70f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b49b70f3010422426c9ab2de1686765a29942c2e))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`bcb5919`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bcb591937ab8491c47e2f1a320b1c440abdff17f))
- update ESP32 build footprints [skip ci] ([`d8c8be4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d8c8be4e80b4566b875cb4ce108f11aa616f58ce))

### Features

- dual-stack IPv6/IPv4 fallback selection (RFC 8305 + 6724) ([`5b9fa5f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b9fa5f5b3f0e5279d771b23621fd088c4106258))

</details>

## [5.47.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.47.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`3cea423`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3cea42314b5c5cb2e0e10aa6c9c06f821be559f9))

### Changes

- Bump version: 5.46.0 → 5.47.0 ([`bb32a2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb32a2b9722b19d0d11e1103185482d15ed9a676))

### Documentation

- update ESP32 build footprints [skip ci] ([`e1a6667`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e1a6667e7fd9f6cc4f67313cf2fd3f73b46539eb))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`07241a6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/07241a6b52773f69e8509758726b948871062f14))

### Features

- DRAM/PSRAM buffer placement policy + SPI DMA ping-pong ([`b4336ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4336ba860cdc585e905082f5d487507c7a8ff15))

</details>

## [5.46.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.46.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`22cc808`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/22cc8089f577b40044e497508495924b82518d73))

### Changes

- Bump version: 5.45.0 → 5.46.0 ([`a105fad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a105fadf80074ec50d5beae357df5288b52fc858))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`1959a3c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1959a3cdfcd116b84e7747e5901a1e31fb8cd8b1))
- update ESP32 build footprints [skip ci] ([`7bd76fe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7bd76fe4018f491611577c190dc68451ff7a83a2))

### Features

- dynamic socket recycling via an LRU connection-slot pool ([`c9c8ab4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c9c8ab4705a2c500cae8e15c6d48bbbbfe07895c))

</details>

## [5.45.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.45.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`17f7e33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17f7e33520668a73b472a37853c60cf4ebdc26a9))

### Changes

- Bump version: 5.44.0 → 5.45.0 ([`2852386`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2852386754b11e6b679ac0a93c2937fc19c0f316))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`10a0f9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/10a0f9c3078ac534babbd20daa78bbd22cd4e4df))

### Features

- RF-aware mDNS beacon scheduling (services/mdns_adaptive) ([`a014212`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a014212d01b050332fb8a7edcae22f4631a3b751))

</details>

## [5.44.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.44.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`397009e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/397009eef30e2c53cfa8f30c44ea052e0c140850))

### Changes

- Bump version: 5.43.0 → 5.44.0 ([`7582c89`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7582c899cbbf70689819fdc1f6aa70f98e76f9f1))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`391b30b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/391b30bdfbca221a4f59300c13313230c98ba1e8))

### Features

- hardware-health diagnostics (services/hw_health) ([`0de74f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0de74f18728ea1b4b78c5176b251782b7736d4c5))

</details>

## [5.43.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.43.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`490f9a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/490f9a7ee50acec77cb90db5ced547f320c70b69))

### Changes

- Bump version: 5.42.0 → 5.43.0 ([`dc8c6e2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc8c6e2e3f1e56603073df4a8fbb01ddfd0e6c1c))

### Documentation

- update ESP32 build footprints [skip ci] ([`f92a301`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f92a30171bb46a7971d2884fca2ba062d0570754))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`6336e83`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6336e8372ee6574a1adfc3fa69c23a20423d4706))

### Features

- stale-while-revalidate + Range delta fetch + SW precache ([`e44aa1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e44aa1ffc889b67311f80d8aa853fe73612f4552))

</details>

## [5.42.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.42.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`e0bfb0b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e0bfb0be38f7581076f07b0c6a9b20288821427c))

### Changes

- Bump version: 5.41.0 → 5.42.0 ([`61e8189`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/61e8189c6beddcf454cc9536eb013f21d91f5490))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`26552d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/26552d1515f1071e1bb77caa507fe9b714f68e1b))

### Features

- ESP32 panic / exception decoder (services/exc_decoder) ([`49f1ae5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/49f1ae5ed8a98c1dd1b8a39eb631a5869afab05b))

</details>

## [5.41.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.41.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`e250f7d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e250f7d08df1c9c31b0f0a1f01f7adaa7952d411))

### Changes

- Bump version: 5.40.0 → 5.41.0 ([`c52096f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c52096f437a9c8c05ce8266795a6db8605686e00))

### Documentation

- update ESP32 build footprints [skip ci] ([`891322a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/891322a4d653d99f46f57b94cec1581b0d789d41))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`4c937ea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c937eaf1a13818202d233f036e92333103bb9d0))

### Features

- southbound protocol-driver framework (services/southbound) ([`75e8801`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75e88010815de1b5d40cf73b1aa7d8eec558ee2c))

</details>

## [5.40.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.40.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`c592f18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c592f182e0c451c9101a3a735835f0b4df4fffdf))

### Changes

- Bump version: 5.39.0 → 5.40.0 ([`fdc8ed8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fdc8ed8db98c5187388937e6852cd656db8b4aff))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`82f56a5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82f56a5eecce61c757902282427f89f7eb9d0cc4))

### Features

- ATC field-I/O interop snapshot (services/atc) ([`1b96362`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1b963629dd7e2d400249f928e02daeb49a676d10))

</details>

## [5.39.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.39.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`61106ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/61106ab663498252824e32d5bb5cffb567c62677))

### Changes

- Bump version: 5.38.0 → 5.39.0 ([`9ec4fdb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ec4fdbc3d66003ce8f264b995bef59112a78ac4))

### Documentation

- update ESP32 build footprints [skip ci] ([`9e3b75a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e3b75a2d8a224441a6b9e8922f8a0f1bb875448))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`fefea36`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fefea36820d335a10c81e3f8a27f82a5007a39c0))

### Features

- OCIT-Outstations road-traffic-control message codec ([`4b6d3cd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b6d3cd9eb4fb8ad8f17f407e9b910302fc3c842))

</details>

## [5.38.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.38.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`a84b3c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a84b3c25dd18ff12f77069e457ceb7cf05662dae))

### Changes

- Bump version: 5.37.0 → 5.38.0 ([`69ada5c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/69ada5ca8b45f6489d38cae23582021ea65bb890))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`2dcce02`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2dcce02b9b02bbe446d2ecfba5b37b2d3b681ca7))

### Features

- UTMC (Urban Traffic Management and Control) common-database codec ([`b9274ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b9274eca3091686478d4eefc9754bce5a5f6f828))

</details>

## [5.37.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.37.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`7779e87`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7779e87c74ba192776a0690b36eeed0fea4a8b00))

### Changes

- Bump version: 5.36.0 → 5.37.0 ([`65a2d2c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/65a2d2c80d0835a1833ed850c7fea2806be34815))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`8b177ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b177ad601f3f3e0975396b20768ada64ba37cc2))

### Features

- IEEE 1609 WAVE (WSMP + 1609.2 envelope) codec ([`0fee617`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0fee617454c61db8338c388492c7fad4e1ef4ff0))

</details>

## [5.36.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.36.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`7545818`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7545818a09ffe41b80f8e10a48b26519142fc415))

### Changes

- Bump version: 5.35.0 → 5.36.0 ([`7478195`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/74781955a554150a4bc94607e253d77df9a89b06))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`a2c0b3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2c0b3f47aa921a71d22baa73df9198c70a420c1))

### Features

- ICCP / TASE.2 (IEC 60870-6) Data_Value codec ([`e2ff9f0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e2ff9f097ceccb4f0d7add50851742d8ec7f2282))

</details>

## [5.35.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.35.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`34289b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/34289b938a594ab7ae76602f721437aae80640c4))

### Changes

- Bump version: 5.34.0 → 5.35.0 ([`cb8a309`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cb8a30914444766f9c54f60f3d6a1d0f1b834b44))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`689fc85`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/689fc859de196a35624bd8ffb1ca5e813b775abd))

### Features

- INTERBUS summation-frame fieldbus codec ([`4088f0c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4088f0c55cc401758278ec6572b5807612b6580f))

</details>

## [5.34.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.34.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`d03e3ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d03e3ee750b2a192fa4b70e4c6106e1aad059a78))

### Changes

- Bump version: 5.33.0 → 5.34.0 ([`2ce7b87`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ce7b87e575c6595e1c4cc4d55f8a5fd062ade91))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`0203cd0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0203cd09d58b957763fb587a42926ca91f558d68))
- update ESP32 build footprints [skip ci] ([`59ece29`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/59ece29a9c2cab1c6f91ac4d5fdae741c8756edd))

### Features

- Modbus Plus HDLC token-bus frame codec ([`014c925`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/014c925a0287bcdb9ca8c1e97d92ffa45a1b27ad))

</details>

## [5.33.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.33.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`b3e5467`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b3e54672de57cf312c1fef909ccb947750c15bb9))

### Changes

- Bump version: 5.32.0 → 5.33.0 ([`1c705ae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c705ae2fa5111f4c66d484c7df2571915fcf74a))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`5f95357`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5f953573e4951a878658eaa442a60df7fbad26a5))

### Features

- LonWorks / LON-IP (ISO/IEC 14908) network-variable codec ([`0b7048c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b7048ca949c491880fe408e0397b867b3fbcb70))

</details>

## [5.32.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.32.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`185c250`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/185c250bf1dfc70ac75ccbb71a3e5f2287b73011))

### Changes

- Bump version: 5.31.0 → 5.32.0 ([`e7e9ea9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e7e9ea9ca5ff0fffece8df5f79ee47a61e386384))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`9075f93`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9075f930a860565113c3b85563a9eb441e2c56b2))

### Features

- PROFIBUS-DP FDL telegram codec ([`fba209b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fba209b1ba46be47b71572d586c3310e51e83700))

</details>

## [5.31.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.31.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`9efb507`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9efb5070721d37bf3eb37fd8b393a35a05ce2c04))

### Changes

- Bump version: 5.30.0 → 5.31.0 ([`35cdee9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/35cdee9d5f56c8978e847932c909114516e7486c))

### Documentation

- update ESP32 build footprints [skip ci] ([`a8306b5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8306b5d620fc2590eba7e865764aa2ef99deecf))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`9fe3885`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9fe388531b619f166ec53a11b81d430966109c66))

### Features

- SERCOS III motion-bus telegram + IDN codec ([`72a80ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72a80ecc8fc8197dc820b9aa83ec6bfff7d9f5a3))

</details>

## [5.30.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.30.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`84394d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84394d76cc187f5f4482fc0cc85e2ea12b3e66f2))

### Changes

- Bump version: 5.29.0 → 5.30.0 ([`defb27e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/defb27e5cf9fe6816e37a2d5255ef14a96d22f5a))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`77011fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/77011fa19f96184300c2275e213ce72feff67e0f))

### Features

- Ethernet POWERLINK (EPSG) basic frame codec ([`8eb5f2e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8eb5f2e9077f35671aa2c8e12b92f327815fc036))

</details>

## [5.29.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.29.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`7cc67c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7cc67c6b528c69d93b749b1023728080fecb6b0a))

### Changes

- Bump version: 5.28.0 → 5.29.0 ([`e94465f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e94465fc53a18289ed35d5f2a588a236f920378a))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`d4c0497`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d4c0497debecbe918ab41be7a4a0a4a8ea678ce0))

### Features

- CC-Link (CLPA) cyclic fieldbus frame codec ([`6ffc643`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ffc643e968798bd8da67ba032c6329ebed98825))

</details>

## [5.28.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.28.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`a7785f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a7785f5a8e3794c842a5964dc2d861d2ab1d0d7e))

### Changes

- Bump version: 5.27.0 → 5.28.0 ([`5a0c7fc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a0c7fcca490384c21a7fdf16fb61250b69e13bd))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`b6a2f6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b6a2f6e184c0d625cc3f7b54a829332ace2ae0aa))

### Features

- IEC 61850 MMS confirmed Read PDU codec ([`1853698`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1853698d70c522a9d7d755d0cf966cfc957c84fd))

</details>

## [5.27.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.27.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`06da0ea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06da0ea6c1e558ede5dfadc3ff25f7ff5ac34668))

### Changes

- Bump version: 5.26.0 → 5.27.0 ([`2b071d6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2b071d622ed296a454e5d65a5c88c24d73872dc6))

### Features

- OpenADR 3.0 (Automated Demand Response) JSON codec ([`7a1baa1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a1baa15f6339d1af6928fde92c657a38be9c0b6))

</details>

## [5.26.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.26.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`aef6c2c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aef6c2c4c70d88a249d5e4ef7b02b93532658a0c))

### Changes

- Bump version: 5.25.0 → 5.26.0 ([`38f74c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38f74c98b2ab11c8c90d3576cac78d3778eb2adb))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`3841885`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3841885fabdbe7e4755583d176c4fcf5846421d7))
- update ESP32 build footprints [skip ci] ([`7385d37`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7385d37ef234fbc3ae00ee2d0d07ba9b0c301229))

### Features

- MAP intersection geometry - completes the V2X message trio ([`232d7a6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/232d7a68570087918a07a1101bdc220d7e71b04d))

</details>

## [5.25.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.25.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`c7589e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7589e1e3c12c4b8c24d4437971befb79dada892))

### Changes

- Bump version: 5.24.0 → 5.25.0 ([`f135b13`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f135b13da7110973db850386cc58477e4fc20679))

### Features

- NTCIP transportation-device object OIDs on the SNMP agent ([`815c271`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/815c2710c309c6ddcde6bd9ba263d0d2a50b907f))

</details>

## [5.24.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.24.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`91f5fba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91f5fbaa1262cc7f28ea74f318eda33fe365d281))

### Changes

- Bump version: 5.23.0 → 5.24.0 ([`a23f6e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a23f6e1c8f8ffec43298f80c6ef6e5fd34bbeca8))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`c30f159`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c30f15951990f054d1998a641ff7904d7bdee2a9))
- update ESP32 build footprints [skip ci] ([`c6d3970`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c6d3970fb9e4dec9759e56a7eee7b83202e33fba))

### Features

- SPaT MovementState (signal phase + timing) on the UPER codec ([`7449517`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/74495177d04cb9813de2f6ab558369382f0e881d))

</details>

## [5.23.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.23.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`7e29b80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e29b80bf7accbe005b574a2e07b9d752bcc4b13))

### Changes

- Bump version: 5.22.0 → 5.23.0 ([`17fc249`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17fc24914f3c2af319f2fe2ae421518f6e338ced))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`ed45104`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed451043ac99137619676af945a616c3908c6f79))

### Features

- PROFINET DCP (Discovery and Configuration Protocol) frame codec ([`eab4056`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eab4056a9457b6c988f2feb5a9ac136f820080a2))

</details>

## [5.22.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.22.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`ac55a60`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac55a6072e8f272bc791711eb499a00fbdbfd620))

### Changes

- Bump version: 5.21.0 → 5.22.0 ([`7a9b4ff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a9b4ff7a04f4a238ad854f41f46ae2b7e6db77b))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`b1a8594`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b1a859492bef42500a3208cb40aab1787bb77e25))
- update ESP32 build footprints [skip ci] ([`3d3b214`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3d3b2144c7a4b26b38b8cbbf0c19a92254ad523e))

### Features

- IEEE 2030.5 (Smart Energy Profile 2.0) resource codec ([`c6a5b68`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c6a5b68af31797025b08c93f87076f2f4ec60508))

</details>

## [5.21.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.21.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`3697907`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36979072cfed23629a76a1e70624bd4fc5fb0bb8))

### Changes

- Bump version: 5.20.0 → 5.21.0 ([`4a2775a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a2775aa983148f11751d20d540213b737204045))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`aa655db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aa655dbc31cc52b222bd0d9a1b76bad23fe064e8))

### Features

- AutomationDirect / Koyo DirectNET serial frame codec ([`439ba11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/439ba116b7807844e373365d76f67dc0788e7491))

</details>

## [5.20.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.20.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`f24c566`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f24c566a9362659bb4ba7baf4b02a80b504f8065))

### Changes

- Bump version: 5.19.0 → 5.20.0 ([`5e67a75`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e67a75743d62df24ad6b6978fe320846d516650))

### Documentation

- update ESP32 build footprints [skip ci] ([`64d0688`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64d0688d910c31ab342c8866fb4bf2e78ebd073a))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`120749a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/120749a68fd60ca923bce22c136cc76a36ed7885))

### Features

- GE Fanuc SNP (Series Ninety Protocol) serial frame codec ([`71af240`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71af240905e6830ed249f99df934174b9b48e463))

</details>

## [5.19.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.19.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`76c9d27`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76c9d2712707f0a0c4f1cfa81dc3301e13b86ba7))

### Changes

- Bump version: 5.18.0 → 5.19.0 ([`4d6475c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4d6475cfd1b8a6b954642dc1d5eb860122dfdd35))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`98d63f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/98d63f18c902e660f206dcccd217ea5601a19712))
- update ESP32 build footprints [skip ci] ([`6cd0152`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6cd0152c1617318ef096f500b9f71a61ce77215d))

### Features

- NEMA TS 2 traffic-cabinet SDLC frame codec ([`1d39c8c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d39c8c1b5440d5c5e41489ed794bc1c69dfabbd))

</details>

## [5.18.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.18.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`a68c6c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a68c6c69912b345ed4431b3bf88a326feb7e3474))

### Changes

- Bump version: 5.17.0 → 5.18.0 ([`f21685f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f21685f2ceca9ac2f92469e2d158764f1e6bc47b))

### Documentation

- update ESP32 build footprints [skip ci] ([`7e083fc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e083fc0ffe43bb45e7902ae1e2e8962716a65c6))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`06547bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06547bf181f25160fa11d099dc0a839c40a4a185))

### Features

- SAE J2735 V2X ASN.1 UPER codec + BSMcore ([`3d869d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3d869d96dcbd231f8acc8f9ab666be2e2a98fe4a))

</details>

## [5.17.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.17.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`8a1b7bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a1b7bf44f0171ae6519933bdc61fd3637dd85aa))

### Changes

- Bump version: 5.16.0 → 5.17.0 ([`f40dd34`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f40dd34f7d3a996dcd58bfa231dcbf5c89dfba33))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`2c6ebbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c6ebbeaa6cbc6bf960d06a3eff3bdc0a2e284b8))
- update ESP32 build footprints [skip ci] ([`1079df6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1079df6897589458e31bcc039e34787542d6613d))

### Features

- MTConnect agent response codec (ANSI/MTC1.4) ([`d18e68d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d18e68d59d889236e006949ad95b6163389dec16))

</details>

## [5.16.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.16.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`a0d1597`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d1597d4fd1e447497c4eff94d4d6d2a9c6a69d))

### Changes

- Bump version: 5.15.0 → 5.16.0 ([`699dbf2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/699dbf2b3e8448481141256d98acca231bc7c98a))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`975a783`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/975a7832309758d7845fd803aa216a55293cea39))

### Features

- IEC 61850 GOOSE publisher codec ([`6ee5506`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ee550638474742d45159dcb0604c639e498ad4f))

</details>

## [5.15.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.15.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`a865067`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a865067e5e2663d9d471fc2f8f13dc7a77ffd662))

### Changes

- Bump version: 5.14.0 → 5.15.0 ([`82777e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82777e942de4fd173ca5bf628c4811a019a99df8))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`e819849`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e81984987cef4209bfa220c98a2077279cf0ad8e))
- update ESP32 build footprints [skip ci] ([`c9afe41`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c9afe41211845df8979f85739ae85fcce8ba572b))

### Features

- single-page-app micro-routing decision ([`8fd8f9a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8fd8f9ac985d244d92e7fa5fc18040a284475ab0))

</details>

## [5.14.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.14.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`2488147`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24881478e1efff97e4085701f529a2d4f173a0ed))

### Changes

- Bump version: 5.13.0 → 5.14.0 ([`a8abc14`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8abc14fc089b730d61a6b5d5a7db3838208dc86))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`8c7dc65`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8c7dc6547f1d5c0162e95b0d25c5721cd94f0661))

### Features

- raw Layer-2 Ethernet frame codec ([`bc13d3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bc13d3f8984db7a2e0d2a21e6777cf1263051082))

</details>

## [5.13.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.13.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`2fca8c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2fca8c9be56eec932ab857afca53779e148742b6))

### Changes

- Bump version: 5.12.0 → 5.13.0 ([`fd72093`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd7209361087d059345e4acf4b549e1dd7e434bc))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`dc3572f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc3572f674ff185e4e9050129efd4df72594d925))

### Features

- XMPP (RFC 6120) stanza codec ([`6eaf205`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6eaf2056d26eac6db8d97cbda6b41b83d948b236))

</details>

## [5.12.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.12.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`e860911`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e860911a369ed4baf64a8af802afdc5ae16250f6))

### Changes

- Bump version: 5.11.0 → 5.12.0 ([`5a87fb9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a87fb9cdee4dd224920ba40727f99d4c9936cb6))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`adf2f42`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/adf2f4270c520047203c6ac32bc29a380defd7fd))

### Features

- DDS / RTPS wire-protocol framing codec ([`b661137`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b6611378ed0ad36041eb579762f365b185c7bbf2))

</details>

## [5.11.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.11.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`1f6bfdf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f6bfdf19763d4a5704da0f1534ad94c5885047d))

### Changes

- Bump version: 5.10.0 → 5.11.0 ([`92863bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92863bc748a87ad7ed90162d38e8f1169dac1896))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`50acedf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50acedf042707fe2fbc7f13336ef7e8a4662bf45))

### Features

- Network Time Security (RFC 8915) wire codec ([`f413b97`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f413b973b3245d5520067366956ec89204c28b2b))

</details>

## [5.10.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.10.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`2c0d331`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c0d33185b3d653f39ba8560ad3f158cbc4ef2a7))

### Changes

- Bump version: 5.9.0 → 5.10.0 ([`76465b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76465b12a4936a226b0497b2832c1d215e6f2a17))

### Features

- HART / HART-IP process-instrument protocol codec ([`d02389f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d02389f107e77b43e1d952d9400471b5765052cb))

</details>

## [5.9.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.9.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`c2acf42`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c2acf427e1e7d0b61fcc691e054283443151365c))

### Changes

- Bump version: 5.8.0 → 5.9.0 ([`ad113c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ad113c2e6a09dc0072de3c218c005236b37bcd06))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`8e34b8e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e34b8e31c668fea2887c757da5677d9fc6f18b9))

### Features

- OneShot/Multishot analog-PWM throttle mapping (+ ProShot note) ([`0550cf1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0550cf1038d79a6894ea4b0cca27bfeb68d03c46))

</details>

## [5.8.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.8.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`2ddf934`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ddf9340adc32fbb240294bef607e1665f65dc3c))
- update CHANGELOG.md [skip ci] ([`1e8ca7b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1e8ca7be52dde1ced4f0045c43330d1dcb8122ec))

### Changes

- Bump version: 5.7.0 → 5.8.0 ([`e69a547`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e69a547e4d68626adf06e3c24e935b65a2cb4408))

### Documentation

- reconcile 4 stale items - CANopen, IO-Link, DeviceNet, IEC 60870 ([`934864f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/934864fd2a2d9563f020d86d077b0af611b28182))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`10dfebe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/10dfebe661d5246dba2060ffb5686b8c2a8d1a1d))

### Features

- DShot ESC digital throttle protocol codec ([`b90fdbd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b90fdbd28370e3c65aafa3b9d78143144bd6e7a5))

</details>

## [5.7.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.7.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`a71db74`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a71db7421e631a0cd7757dae49d14a03fba1d54b))

### Changes

- Bump version: 5.6.0 → 5.7.0 ([`e49085d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e49085d05320376a9539e4e4a8edf807f599f543))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`79ebe65`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/79ebe65a601343c3091ce68f09d5424a63071c58))
- update ESP32 build footprints [skip ci] ([`54895b5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/54895b5a52b46e4aa5bdc0ccdb1b51be2f1b0d50))

### Features

- TCP window sizing by free RAM + DHCP->static fallback ([`591b8b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/591b8b6998d90b63ad01ad24fcee815052a56a1e))

</details>

## [5.6.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.6.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`9238268`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92382680795a926748978984a2ee4ad25bf86e0c))

### Changes

- Bump version: 5.5.0 → 5.6.0 ([`1889570`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/18895707bcfd43df4cd0338b07fdb685bae6ec26))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`b85b932`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b85b9323e9d96aa3f576b378780b2b0ca9e230a4))

### Features

- flash wear-leveling slot selector ([`8c1860e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8c1860e074df6523928f9a1a5dc3ef0990158038))

</details>

## [5.5.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.5.0 - 2026-07-06</b></summary>

### CI / Build

- update test report [skip ci] ([`2d53f88`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2d53f8838dc7c4eb303973db48f9d3770d71046a))
- update CHANGELOG.md [skip ci] ([`09dd3ac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/09dd3ac2d3eed43ca15d4c6e2cf60d722b9e5737))

### Changes

- Bump version: 5.4.1 → 5.5.0 ([`3d173c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3d173c609b4fe9be6ab3501900533ffd0e721f92))

### Documentation

- update ESP32 build footprints [skip ci] ([`a1b200d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a1b200decb8191aa83f18d8b440dded29953bb76))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`3f9a837`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f9a837cec57f2c97982266c893ad2b6abfb0c88))

### Features

- dynamic sleep-cycle scheduler for low-power nodes ([`5ed1dd6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ed1dd639bf205ef3671c5f08c595a9a31b4649c))

</details>

## [5.4.1] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.4.1 - 2026-07-06</b></summary>

### Bug Fixes

- make the memcpy bounds unambiguous for the SE analyzer (S3519) ([`b0e0206`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b0e02065b5697841387865df8136010d0bd3485d))

### CI / Build

- update CHANGELOG.md [skip ci] ([`8e1bbfe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e1bbfe7e67d043f9424b067db64322891bc419e))

### Changes

- Bump version: 5.4.0 → 5.4.1 ([`bdf1c89`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bdf1c8931ee920e528108d78e8db1af6c5aa9deb))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`034b51d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/034b51dc1d4d4a2679e8dcff91b884319fd10541))
- update ESP32 build footprints [skip ci] ([`9712bb5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9712bb52927c44652bf02eeb92fb59183795aa2a))

</details>

## [5.4.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.4.0 - 2026-07-06</b></summary>

### CI / Build

- update test report [skip ci] ([`294ea3b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/294ea3b7977bd1f847574f3ff727a239ac9ffe45))
- update CHANGELOG.md [skip ci] ([`4b817f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b817f194e09ed21d62bdec9b79b1109ebbd735e))

### Changes

- Bump version: 5.3.0 → 5.4.0 ([`86ce0a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/86ce0a3da4cf0253f06b53a7d29a9e0c49ccf5e8))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`8315b42`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8315b4202db62287d9f26d109e83c43c897ed0be))

### Features

- software watchdog - deadlock detection + fail-safe safe-state ([`23bee33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23bee33e8f1c1036a6451ddd06607a1d9b5162d2))

</details>

## [5.3.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.3.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`39eefcd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/39eefcd63f9195396b7cef9cd63f44410bc7a63e))
- ccache the xtensa toolchain for the Arduino-CLI example builds too ([`4ae0f7e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ae0f7e5a3eccaac505aebcbed334ad982411edf))
- update test report [skip ci] ([`aff2cc0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aff2cc0402b3d4b1cfa5455d3c9f2e16f4b1f9d0))
- update CHANGELOG.md [skip ci] ([`b626c2e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b626c2e9dcada4a59fc344a16f16008c24ce98df))
- ccache the xtensa toolchain so examples reuse the Arduino-core build ([`08e4bdd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08e4bdd69bdf8b92f52072bce24c5bf11d211fc4))
- update CHANGELOG.md [skip ci] ([`71341d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71341d4dd22dce380426f3c069b4d5a42a87a4d8))
- run the native suite once for report + coverage; ccache the compiles ([`8c36195`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8c3619514c791f46edce249f312a023bfc03e5e0))
- update test report [skip ci] ([`e0461d3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e0461d3967bb3a9f4cf6b7f910f9eb439ae18ed6))
- update CHANGELOG.md [skip ci] ([`847269b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/847269bf5ba6a7e05e3d32cf77e546c0653b88e8))

### Changes

- Bump version: 5.2.3 → 5.3.0 ([`bbd40eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bbd40eb52291fb5d2e84fb65c85a9240c42e8797))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`d6c6e56`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d6c6e567f4c062f5815ad3e35f13b1818a86d156))
- update ESP32 build footprints [skip ci] ([`fe1e9fc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fe1e9fc40f562bf47f06096ec93af91577e28c0a))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`7e3448e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e3448e47ac06af74548b85d22a73bdf7fe81218))
- update ESP32 build footprints [skip ci] ([`f4be549`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f4be549fe6a815f1e3762331541befdb1397387b))

### Features

- outbound MTU-aligned fragmentation (RFC 6455 sec 5.4) ([`c4e9611`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4e96117275dd2dc9eb73b60f9bb4ef0dac30230))

</details>

## [5.2.3] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.2.3 - 2026-07-06</b></summary>

### Bug Fixes

- make footprint budget row order deterministic (stop per-push README churn) ([`a908c4f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a908c4fa9f9d9f850a642bf98019578f94d51d77))

### CI / Build

- update test report [skip ci] ([`ae8d11b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae8d11b5f6b8248b6741e213c345466e558f3761))
- update CHANGELOG.md [skip ci] ([`4f5e5ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f5e5ef5131f7540b501ac0eab1fff8bbaf9d3d2))
- update test report [skip ci] ([`c969dae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c969dae3ad83a0f392ef830102823ee19c009fca))
- update CHANGELOG.md [skip ci] ([`04fba3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04fba3f8a937bdbc10f1e5163bb2f613cdf7edbd))
- update test report [skip ci] ([`0177116`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01771166a6573349febe45559df114033c323bf8))
- update CHANGELOG.md [skip ci] ([`b17859d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b17859da0011e1cba7ea70a1fc2ec857504bd0b0))

### Changes

- Bump version: 5.2.2 → 5.2.3 ([`c538861`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c538861c618cc203ef53220aa18cdb335fe0580f))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`a1867ac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a1867ac4e76663b04321abc2bca2de9b8079eda6))
- complete per-feature footprint budget from the full 95-feature RPi matrix ([`4f6c7ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f6c7ef9e39247b9eabc48a3af32e5d5856cc60f))
- update ESP32 build footprints [skip ci] ([`bdcca10`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bdcca10db1f6a6826205be38874f1c6fe2b8eea7))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`f270386`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f270386384ec37b8358f4e6664cc7fb5e14d932b))
- update ESP32 build footprints [skip ci] ([`73d24eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/73d24ebacfb273dcd188d133dfb14595147c35c2))

### Refactor

- route HTTP through the uniform ProtoHandler::on_poll seam ([`a56fe4e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a56fe4e29e4fdcf01b4e7e890bda7508ce1031c3))

</details>

## [5.2.2] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.2.2 - 2026-07-06</b></summary>

### Bug Fixes

- guard w_bytes pos<=cap explicitly so the bound cannot underflow ([`3f2b418`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f2b41867154d7c94900393482978a1d895b0f98))

### CI / Build

- make changelog + feature-tables auto-commit rebase-safe; skip unchanged diagram renders ([`e821b3e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e821b3e3afd4095a6f53f7f4e4547c930a219805))
- update test report [skip ci] ([`f25a3b8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f25a3b8032fd5d7c987ca0ef4f1a82a3ce804375))
- update CHANGELOG.md [skip ci] ([`f22dc24`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f22dc2497d5d1ac5b5ac38cce1f8d32857b2a1ba))

### Changes

- Bump version: 5.2.1 → 5.2.2 ([`5a27bb5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a27bb5b75c00dfee0a33c56abeda400f4232200))

### Documentation

- log the HTTP/3 flight-cap overflow fix (v5.2.1) ([`bd63100`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd63100ac4a5728110b937693ed83c3241b94ae8))
- update ESP32 build footprints [skip ci] ([`5a6055f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a6055f1d80ea8a323880b291216757e881cabb6))

</details>

## [5.2.1] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.2.1 - 2026-07-06</b></summary>

### Bug Fixes

- honor flight-buffer cap in emit + overflow-safe DATA clamp ([`85c7b1e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/85c7b1ea5c4f17df222cac15ec54d93c2822258b))

### CI / Build

- update test report [skip ci] ([`338509d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/338509d19fe0db86c152916744a9242052a937e1))
- update CHANGELOG.md [skip ci] ([`f306d03`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f306d03075c042dba9d50f83e18767dc5beffa44))
- update test report [skip ci] ([`f67d7e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f67d7e95405b0161a6447946f5f236339593ad57))

### Changes

- Bump version: 5.2.0 → 5.2.1 ([`2e82707`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2e82707d7ea130481372184dc071433b83075877))

### Documentation

- update ESP32 build footprints [skip ci] ([`6e4c71e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6e4c71eb92a2bd1e65de6a5a5129c04ed15009c5))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`06d1d6d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06d1d6d36c9d0fd8ff5d8c2df1f1214203a7ab7d))

### Features

- gate trademark-named themes out of commercial builds (keep them in OSS) ([`8334989`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8334989086e4ab8d8f8f74cbe03f440842d3512d))
- expand theme library to 112 + wire theme/favicon generators into CI ([`e6dfd84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e6dfd84b1a462cc19194424d29c0827cb2c9b21c))

</details>

## [5.2.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.2.0 - 2026-07-06</b></summary>

### Bug Fixes

- check anti-amplification before building, not after (flight desync) ([`34aa5f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/34aa5f834231c6db478faf2080bdc429d1e9de7b))
- reset the PTO backoff on acknowledged progress (RFC 9002 sec 6.2) ([`77f723d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/77f723dc9ef426dfb6837539616a9e4c14aba62b))

### CI / Build

- update CHANGELOG.md [skip ci] ([`09f9e14`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/09f9e1402f68fea7972acd4b2bac9179c786e63f))
- update test report [skip ci] ([`3fa7377`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3fa7377fe3d9f7c4a94f30793bf90790ade81270))
- update CHANGELOG.md [skip ci] ([`c357d3b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c357d3bcf3ee4386453866113724ccfaeb439a77))
- update test report [skip ci] ([`5b80427`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b8042705c2c21609d1a49824ded0653f30b2747))
- update CHANGELOG.md [skip ci] ([`2341d00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2341d00a4bf339f300f68cb4fa6f8fd72f752e53))
- update test report [skip ci] ([`5c21807`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c218079e5bc5fe99ee0950e2db3164a5e892f1f))
- update CHANGELOG.md [skip ci] ([`a40f9ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a40f9adf88fe5d1692c46c772ccdb80c40966714))
- update test report [skip ci] ([`f3cb9ae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3cb9ae73c0ca012d86766c0480db8881d31e106))
- update CHANGELOG.md [skip ci] ([`e5a9f71`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e5a9f71c2f5deeb01bd5cb1819c6ccdab3a0a7a8))
- update test report [skip ci] ([`f14c9b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f14c9b67d1d343f7ad19b87becaee7380366cf39))
- update CHANGELOG.md [skip ci] ([`8449fb7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8449fb76a7040040625f8f59dba7b4cf3c2fe13c))
- update test report [skip ci] ([`0e926bd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e926bde107e26cea9e23669111a7be1b2d3ae1a))
- update CHANGELOG.md [skip ci] ([`d2db8c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2db8c219000b284ccd0b9453d10c20d7e1f8318))
- update test report [skip ci] ([`30d53a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30d53a2184a27b8177d184a640947325d7242bb1))
- update CHANGELOG.md [skip ci] ([`bc6261e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bc6261ed957051ca9c1bce0ace48679ae2c8d1d5))
- update test report [skip ci] ([`b4c7bc3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4c7bc33cbfe2d76154f7efbcb067a994ef216d8))
- update CHANGELOG.md [skip ci] ([`4db78d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4db78d464706b500259f949b027526429abdb808))
- update test report [skip ci] ([`cd1574e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cd1574e434d18fbc976f36f0522809e226e8fcb7))
- update CHANGELOG.md [skip ci] ([`a99957c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a99957cedbb0c45e2f290973061c85a61375f8f5))
- update test report [skip ci] ([`5accd88`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5accd88aae48744de2d9405298cd5c8c466ccd3c))
- update CHANGELOG.md [skip ci] ([`a32a881`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a32a881c4a547abb37654d4147faaff934869b14))
- update test report [skip ci] ([`cf94d4a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf94d4a2e89ac1e4edd355a1658f7e3ed533af3d))
- update CHANGELOG.md [skip ci] ([`b6a076e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b6a076e62cce685eacefd95d96a10eabfef7b5f2))
- update CHANGELOG.md [skip ci] ([`5b3fecf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b3fecf987a796b292519450e7c8e182b0bc8559))
- update CHANGELOG.md [skip ci] ([`0f4f5a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0f4f5a0c7812066fbde5e34fe2834975e5efbbb9))
- update test report [skip ci] ([`321f006`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/321f006cc104c90d9d2a18d01a7cd0b4bdbc054b))
- update CHANGELOG.md [skip ci] ([`f8fe0fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8fe0fadef1993013ab709c9668f9fb6567d48f2))

### Changes

- Bump version: 5.1.0 → 5.2.0 ([`65ad53b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/65ad53bfa4bd857d80338f4a518274cf20b2890d))
- Revert "feat(http3): re-send the handshake flight on a duplicate ClientHello (loss signal)" ([`3824cf9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3824cf99e902c759b9f872e958ee8299060d53f5))

### Documentation

- update ESP32 build footprints [skip ci] ([`4878ada`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4878adaa4fe0b2d994befdd9aafc078bc1d54c92))
- theme gallery + 68 PNG previews + a custom-theme generator command ([`12cb3c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12cb3c508c94a24f68d18e4e1bff61cd6bb518ec))
- embed diagrams as pre-rendered PNGs so they show in the app + Doxygen ([`d2ad529`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2ad529cb73be60b8153d36e470c7e99ce8561fd))
- emit a fully-detailed lifecycle diagram into the architecture doc ([`81f4ac5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/81f4ac5212d727371badee61a5b5c5aa6da25be8))
- theme-adaptive translucent palette + thicker links ([`952872e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/952872eff2a7c6e2fd409f8d1c5452cfa6e94d87))
- redesign the README flowcharts to be beginner-friendly ([`f5862de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f5862de5a7323fd222212eacc308b055a6b5710e))
- log the QUIC anti-amplification build-then-discard desync [skip ci] ([`57c39c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/57c39c280f16188afccb8b68881085d0a2c5a771))
- add final item - Sphinx over Doxygen + squirty styling [skip ci] ([`baf558d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/baf558d757d4f6f15b9db9be22c23c1ee4c1063c))
- update ESP32 build footprints [skip ci] ([`c9d3f7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c9d3f7f7bb15b4bd0a3af4985b8182abf8c4ac25))
- update ESP32 build footprints [skip ci] ([`8da56e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8da56e5881687a86eb6e94d2d97e9b255c5f0eb4))
- update ESP32 build footprints [skip ci] ([`d3d3836`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3d3836b87c13d76dea0a321d50da5d6da5afd0c))
- update ESP32 build footprints [skip ci] ([`a237736`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2377369276d9b34f10efeac930743785bb5a92a))
- update ESP32 build footprints [skip ci] ([`b2bbf18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b2bbf18fb5f39c5944e4cea6cbc17d96ff960927))
- update ESP32 build footprints [skip ci] ([`cf9a59d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf9a59db260984e9696d944b768cc0796159e532))

### Features

- per-feature footprint budget table (best-worst flash/RAM), replacing 3 stale tables ([`3831183`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38311830bcb9cdb3668a7a1da3165deb0269a422))
- 288 favicons (18 motifs x 16 palettes) + gallery + tarball packager ([`4551f09`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4551f09f6fe8ae467b473afb4013986214772620))
- toggleable theme blobs in Layer 7 (DETWS_ENABLE_THEMES) ([`dd02a26`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dd02a2683a04aaf4bca0d8c03eff5e91605d4ab1))
- theme library generator + 61 curated themes + a custom-palette tool ([`d12e883`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d12e883e21a0d3cb7b309dbd0cf0966b9eca4a5e))
- send CONNECTION_CLOSE on a fatal handshake / transport error ([`79fc358`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/79fc35866426e8b2e3996f0318f59dcac219431e))
- re-send the handshake flight on a duplicate ClientHello (loss signal) ([`a651d81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a651d8177854e1ba4b887c76cdbb25572fd53814))
- generalize PTO loss recovery to 1-RTT (retransmit the response) ([`336736a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/336736a4680d8c6e33287829c5b73a30e20e00f1))
- PTO loss recovery - retransmit the handshake flight (RFC 9002) ([`d39d183`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d39d183563daa1845f3313da100d1a7435e237fb))

### Refactor

- uniform per-connection response sink (TX seam) ([`1a12d2e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a12d2e80f51cd263f8522366f290e1cbcee5cc8))

### Testing

- poll the h3 harness on a timer so PTO fires without inbound traffic ([`3817028`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38170288fe0fa6f02bbbf4dc3ae405d49ad292e6))

</details>

## [5.1.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.1.0 - 2026-07-06</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`4365243`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/43652439158e0017ad199994cbbac6f15d7368eb))
- update test report [skip ci] ([`874667d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/874667db1541a03a468e639530c4b4ea25e05f56))
- update CHANGELOG.md [skip ci] ([`8f3e53b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f3e53b2f829edd813c1b3bb52e2c11f2b1b1ef8))
- update CHANGELOG.md [skip ci] ([`1e706ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1e706ab47ca008faa38058d6ab8a648c94f49db1))
- update test report [skip ci] ([`36c1182`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36c1182b088e98f02fc3108f0f5112cb81c5d1ee))
- update CHANGELOG.md [skip ci] ([`bd5c82e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd5c82e9900b558f852215ead8c7dbfe602ff2b3))
- update test report [skip ci] ([`a44432f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a44432f0515e2d5e524dcf82fff6505c36d1f099))
- update CHANGELOG.md [skip ci] ([`cc1559d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cc1559df9e9c6a10a43641538daf9f292fdf93f5))

### Changes

- Bump version: 5.0.0 → 5.1.0 ([`9e74547`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e74547e0dcd29eaf2ce0260d2d640c5ad0bb6e1))

### Features

- quic_server idle-connection reaping + PSRAM build guard (v5.x hardening) ([`ec5679a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ec5679a58176a9623e1d79a78af2bfd659ba51cf))

### Testing

- h3_conn split uni-stream varint + field-truncation coverage ([`122eaec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/122eaec11873b9cf198a3e5cff4025c07e748550))
- cover h3_conn client uni-streams + malformed / overflow paths ([`d50b2b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d50b2b3c323dbfcb8a6b1a3e54be513bd80c68d7))
- cover TLS 1.3 handshake-rejection + transport-param reject paths ([`cbbf1b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cbbf1b19ccfaa1ff6686d5f256d798761b1e4e6b))

</details>

## [5.0.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 5.0.0 - 2026-07-06</b></summary>

### CI / Build

- update test report [skip ci] ([`39a33e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/39a33e42d51e3e916d0252d85b1590f67a3b02a0))
- update CHANGELOG.md [skip ci] ([`375f1f3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/375f1f3b9f03f261e821763f81ef76a571688115))
- update test report [skip ci] ([`b15cf00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b15cf00ef43273b30ffc2a405487e26f38b0284b))

### Changes

- Bump version: 4.129.0 → 5.0.0 ([`1066ad4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1066ad44327d9b25ed2e986a4f5376c4f9fec834))

### Documentation

- render the Mermaid diagrams on GitHub (stricter parser than mmdc) ([`75ead4d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75ead4d69f7c3474768600858803ceb1880c441e))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`9ab58e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ab58e5f184b074562cbaba3c6318ab8cffc9245))

### Testing

- real-client interop harness + mark HTTP/3 complete ([`0b19616`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b19616d55993499a56f48c836f26964411ed6b7))

</details>

## [4.129.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 4.129.0 - 2026-07-06</b></summary>

### CI / Build

- update test report [skip ci] ([`f22ac9a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f22ac9a8705da5297e43e3173efbb0a4c2ee38d2))
- update CHANGELOG.md [skip ci] ([`7cb075f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7cb075f9424612ba9878ce9c8b638d3c68bb873f))

### Changes

- Bump version: 4.128.0 → 4.129.0 ([`33d19c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33d19c5ebb3342f915c799a03aeedd2f7f09453b))

### Features

- serve HTTP/3 through the shared DetWebServer route pipeline ([`d83c1ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d83c1abbd8f51e3879c7ece638b0d1301de80118))

</details>

## [4.128.0] - 2026-07-06

<details>
<summary><b>Show Changelog for version 4.128.0 - 2026-07-06</b></summary>

### CI / Build

- update test report [skip ci] ([`91c926b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91c926b9d1d607f9c80ff99bdf14fd8d9e0b93f2))
- update CHANGELOG.md [skip ci] ([`8d8600d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8d8600d13d7fec00e91898cc9d6c6524460b1e8f))
- update test report [skip ci] ([`d12672c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d12672ca3d48e30673c5a096ee495ddbafca1ad6))
- update CHANGELOG.md [skip ci] ([`990d65a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/990d65a754c42b8bf7fc225f304bc79d5a1f4bdf))
- bump cspell from 8.19.4 to 10.0.1 ([`61d2b6a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/61d2b6a4e3ed835fa8bc14589101f20997ea8c7c))
- post-commit auto-merge of mergeable Dependabot PRs ([`d7484bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d7484bf462a26d3c700956e55db1b8650802f9f8))
- update test report [skip ci] ([`b9ad08a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b9ad08a77a8d64295b82a95e228c1b8ca73b5a35))
- update CHANGELOG.md [skip ci] ([`754c2d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/754c2d71316f33f408784941b3de2a1dc52b9cb5))
- update CHANGELOG.md [skip ci] ([`8a13db7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a13db757eeddee700021ca1dbe2d65690d3ec15))

### Changes

- Bump version: 4.127.1 → 4.128.0 ([`759af2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/759af2bc9f938af073198d7f92c04887af50e192))
- Merge Dependabot #12: chore(deps-dev): bump cspell from 8.19.4 to 10.0.1 ([`c5e2145`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c5e2145495a30eaea68295f4952309235fb458ed))

### Documentation

- auto-generated build-flag dependency + core API-flow diagrams ([`ff341f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ff341f17a304941cece939c6c94db8c45571b2dc))
- update ESP32 build footprints [skip ci] ([`cb8b8ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cb8b8ba3b72242108d3f269f961c084408d89018))
- HTTP/3 stack built + host-tested (QUIC + TLS 1.3 + HTTP/3) ([`6c6497c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c6497c62ee1da5a535c0cf8d005ba38f9ee0a81))
- update ESP32 build footprints [skip ci] ([`0a1c7c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a1c7c1ec0c15294c5cf8c095efd2028b3d0fdee))

### Features

- quic_server - UDP-facing pool that binds QUIC + HTTP/3 to the wire ([`4c40fe7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c40fe7e5aa2f0cebdffa1ebf485acd52c7b5f8f))

### Testing

- end-to-end capstone - full QUIC handshake + HTTP/3 GET ([`04e2ec7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04e2ec79a9ca9eab77a8309a4f3b158ae47eca0a))

</details>

## [4.127.1] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.127.1 - 2026-07-05</b></summary>

### Bug Fixes

- overflow-safe capacity checks in the TLS/QUIC serializers ([`d71461a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d71461aa33a15028421481543477e4b73813d3ca))

### CI / Build

- update CHANGELOG.md [skip ci] ([`7dff034`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7dff034781b309ac83fa0ed5312d51127c7a42a1))

### Changes

- Bump version: 4.127.0 → 4.127.1 ([`d9b1200`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d9b1200c8a83d8cb6cd70df46d2e202308975e97))

### Documentation

- update ESP32 build footprints [skip ci] ([`5e7cf04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e7cf046bb3c034b954d938dca5ef0df77c681e0))

</details>

## [4.127.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.127.0 - 2026-07-05</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`712a816`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/712a816dd203543a8f68f6155bbd43627ba43b10))

### Changes

- Bump version: 4.126.0 → 4.127.0 ([`af07320`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/af07320927255aea2949900a5dbe411d37cbd707))

### Features

- HTTP/3 application engine over QUIC streams ([`c2c3a04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c2c3a04a8c9151193acf5e382e93d817d4d5688c))

</details>

## [4.126.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.126.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`d0fe4d2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0fe4d2dd077c5dbb852e896f86d429e0d2e07a2))

### Changes

- Bump version: 4.125.0 → 4.126.0 ([`ee0c202`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee0c202d4130965284958d183d6d26c942d8539c))

### Documentation

- update ESP32 build footprints [skip ci] ([`a2a594d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2a594dd65d365a9eb03a283b8a01b5a0c99f6b4))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`d07426c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d07426cc7886d71de710636361987ad8feeb4bb8))

### Features

- stateful QUIC v1 server connection engine ([`a121025`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a1210255aa97b0fee9a77b12c4435442c4bdcc84))

</details>

## [4.125.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.125.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`61e929b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/61e929b882ba4a193db538d76aaae991c63667de))
- update CHANGELOG.md [skip ci] ([`2837ff6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2837ff6588b8a75d06b539b5d0ff95ebb93b81e0))

### Changes

- Bump version: 4.124.0 → 4.125.0 ([`14547ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14547adecd8be54f3ae0bf0a15d6786414ee2b8d))

### Documentation

- update ESP32 build footprints [skip ci] ([`90d1157`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90d1157cceb7facec0710efcd9d0060084f11ba3))

### Features

- TLS 1.3 server handshake state machine for QUIC ([`28063b0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/28063b0f0ff0ec9dafccc677c020514d358b7e70))

</details>

## [4.124.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.124.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`0d51655`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d51655f6ed9eed71a1be1521bbcd8a28e63e176))
- update CHANGELOG.md [skip ci] ([`55fa3b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/55fa3b1353023d815ed308dde7d864ba5712a850))

### Changes

- Bump version: 4.123.0 → 4.124.0 ([`fc42e53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fc42e53e5876f2a26451b6f13c4097a168e09f47))

### Features

- TLS 1.3 handshake messages for the QUIC handshake ([`be979df`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be979df4d45df132f52fb4ad505b18481bba8e74))

</details>

## [4.123.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.123.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`6216035`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/621603532a032c24dd17f8acf9742eda02bcdb44))
- update CHANGELOG.md [skip ci] ([`f8ab021`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8ab021f28f3176b66e41a46d927764282127a81))

### Changes

- Bump version: 4.122.1 → 4.123.0 ([`db6bdbd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/db6bdbd954b52039648f5df06fafe77e55590dc7))

### Features

- TLS 1.3 key schedule + QUIC transport parameters ([`eb0d828`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb0d828dfe8bfeca4b10fda9ac8b32eecaa41651))

</details>

## [4.122.1] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.122.1 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`89a92d2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89a92d2c503c33aad5ad5f079e70b8eb2c44e4db))
- update CHANGELOG.md [skip ci] ([`9ad2545`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ad25450c9951e752778a8abc6ac019098d1fc57))

### Changes

- Bump version: 4.122.0 → 4.122.1 ([`20a3179`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/20a317982cf7fc6a84b62156256b2c37f2877514))

### Documentation

- describe HTTP/1.1 + HTTP/2 (and HTTP/3 in progress) consistently ([`1b724b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1b724b9b5d6d7ef78b2a32c29dc8a337a3351726))

</details>

## [4.122.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.122.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`00758a5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00758a56198722a0cfab0eebcdf8ff8db980a4b5))
- update CHANGELOG.md [skip ci] ([`c8b56d2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c8b56d2019b2dd0b7eaf6d6884dd5bdf8c3fd23d))
- update test report [skip ci] ([`474ab25`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/474ab2547fd31ff4340e45ac2741073403cfe190))
- update CHANGELOG.md [skip ci] ([`1526b80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1526b807198302751ad802cb4750070fad3b6743))

### Changes

- Bump version: 4.121.1 → 4.122.0 ([`4f75e61`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f75e614712037b64851dee0b2d09b361ca8528c))
- Bump version: 4.121.0 → 4.121.1 ([`44d3184`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/44d31842045586dce063848ac449ea6fe73e20a2))

### Documentation

- update ESP32 build footprints [skip ci] ([`1c522ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c522ecbec2109841889c5017484dc7541117ee1))
- update ESP32 build footprints [skip ci] ([`78edf39`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/78edf39d97f1761424913e883db8b04630e989ca))

### Features

- QUIC Initial packet crypto (RFC 9001) ([`64c51f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64c51f7e257fbce496a188f3441965a4616addb7))

### Refactor

- make the Layer 5 dispatcher protocol-agnostic ([`223e678`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/223e67845ad7105acbcef78de1f42a815f161944))

</details>

## [4.121.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.121.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`5bba86c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5bba86c0b828508d5dce9bfebe0c473824a21b63))
- update CHANGELOG.md [skip ci] ([`83d282a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/83d282a6664bd6896a064da5fc0db17cac883f74))

### Changes

- Bump version: 4.120.2 → 4.121.0 ([`d326d91`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d326d91408dc9099ffbbc82a229f1ec6fd3ddd39))

### Documentation

- update ESP32 build footprints [skip ci] ([`50f96fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50f96fa8f5ca3f24961ecea61280fed52021a73e))

### Features

- server-to-client zlib compression (zlib@openssh.com / zlib) ([`35281bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/35281bfb23110372badefe20d88f82672f02287a))

</details>

## [4.120.2] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.120.2 - 2026-07-05</b></summary>

### Bug Fixes

- readable web-asset HTML source + clang-format the ssh test files ([`7fbf4e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7fbf4e0dff9ed8070fa52859714305cc9382d943))

### CI / Build

- update test report [skip ci] ([`aa9c813`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aa9c813daf9e84719746bbc76a852712c5bf0d1e))
- update CHANGELOG.md [skip ci] ([`4e5b785`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e5b78506ff31ed7fab1646c472244cb83b725ee))

### Changes

- Bump version: 4.120.1 → 4.120.2 ([`e470cbf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e470cbfae5502454813cb422a2c33e99bde635cd))

</details>

## [4.120.1] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.120.1 - 2026-07-05</b></summary>

### CI / Build

- black is the default Python formatter ([`1e77800`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1e778004f89b86731356e4d454e6ca9b5d730d30))
- update CHANGELOG.md [skip ci] ([`980d696`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/980d69671fe4318894e3f11cf3f3f340e2ad224b))
- pre-commit auto-fix hook + reformat ssh includes ([`2205921`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/22059216fb765aed0b2088ae490a4eb295fcf598))
- update test report [skip ci] ([`7fe4ff3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7fe4ff3453d337caa9fd5fc3bf01aab9a88ec22d))
- update CHANGELOG.md [skip ci] ([`77cd733`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/77cd733b5cbe8aa532d7a26cdfc8e5b4e9b934b5))

### Changes

- Bump version: 4.120.0 → 4.120.1 ([`5d99fc9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5d99fc93ee1e1239e15679e76d2db6082ef0db18))

### Documentation

- update ESP32 build footprints [skip ci] ([`0fc91a5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0fc91a5f95f91ef8382868988d16f934aba71747))

</details>

## [4.120.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.120.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`37d6923`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/37d6923f230ea7cc9d09ed43461942df12b9892e))
- update CHANGELOG.md [skip ci] ([`eaa0cd8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eaa0cd80316d7841dc500fcf507dba71dbaf72ea))

### Changes

- Bump version: 4.119.0 → 4.120.0 ([`dc50326`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc503260fa064be35ff2093a8d77b8da898c4213))

### Documentation

- remove the vestigial MIGRATION.md (we don't support migration) ([`a4d2b98`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4d2b98e6d5b48cc3e4c5fda5d148525b6b50c18))
- update ESP32 build footprints [skip ci] ([`c2cfd77`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c2cfd77c91ba8de28ded457296452f8dc1cc74a2))

### Refactor

- organize the ssh/ tree into RFC-layer subfolders ([`45b2444`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/45b244409cf5463889b5e97348d75658a64d1f58))

</details>

## [4.119.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.119.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`a9e1426`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a9e1426d033a13c79da3b77a33145705e1d6a0f7))
- update CHANGELOG.md [skip ci] ([`b7e7623`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b7e7623bbebd41823878c5691c2e1dbe5abd624f))

### Changes

- Bump version: 4.118.0 → 4.119.0 ([`995e474`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/995e47423373bdc479c9a619f3b555c14d39f20d))

### Documentation

- update ESP32 build footprints [skip ci] ([`bed4dd6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bed4dd6c3e6c5921e2407dcd693d0215d3e84dff))

### Features

- hmac-sha2-*-etm@openssh.com MACs for the aes cipher ([`fa6681a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa6681afc9075de9a05cf1cb4580685be0273b3b))

</details>

## [4.118.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.118.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`6a1c3dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a1c3dcbf429342de1a9b3687b3d992bfe6d1df2))
- update CHANGELOG.md [skip ci] ([`d55948d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d55948d6e6b5040cdc115c7e8c34422d24793614))

### Changes

- Bump version: 4.117.0 → 4.118.0 ([`8f23997`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f2399747675e7c8337c834022d7cf650321613e))

### Features

- negotiate chacha20-poly1305@openssh.com (OpenSSH's default cipher) ([`7317415`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/731741551cbf76e3702e5ecb8db89702d8dd389f))

</details>

## [4.117.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.117.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`bb31672`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb3167293057bf3390dc18b8f458b9a826745f54))
- update CHANGELOG.md [skip ci] ([`3e30bff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3e30bfffc5716a741e833d41c985d893e7e74e37))

### Changes

- Bump version: 4.116.0 → 4.117.0 ([`9ecabe8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ecabe88a828960026796929d0973a806aeeeca9))

### Features

- HMAC-SHA2-512 (RFC 2104 + RFC 4231), for the -etm/sha512 MACs ([`3d8319f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3d8319f816743e55ffa5869dfaea01b5a6d6200e))

</details>

## [4.116.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.116.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`bd390b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd390b2431dda2fe926457d571065a06fdbf3279))
- update CHANGELOG.md [skip ci] ([`a2e0336`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2e0336edb8c0b8c317ee7c19ffbe2067f45d2ed))
- fix the 5 genuine SonarCloud nits (uppercase literal suffix, ps1 whitespace) ([`e05ba8a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e05ba8a505f43a0fe07fd311df35b2f426fd0814))
- update test report [skip ci] ([`363200f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/363200f00f96dbc9a794b785423f40e670784753))
- update CHANGELOG.md [skip ci] ([`374cabe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/374cabe00b3026a2e4cc82d3fea50acd5da70bcd))
- update test report [skip ci] ([`bb2c02e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb2c02e7f9dbbe104c74482a42d78efb54929fe6))
- update CHANGELOG.md [skip ci] ([`bb413f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb413f84c02fde7c9db7f1ed20474771740996d8))

### Changes

- Bump version: 4.115.0 → 4.116.0 ([`68d4f1e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/68d4f1e0c72a0dff2fbe49c95532edbaf0611439))

### Features

- chacha20-poly1305@openssh.com cipher (codec, host-tested) ([`e7fd623`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e7fd623d20f3796ebbad79680ccc26b64bec9349))

### Testing

- raise QUIC / QPACK / HTTP3-framing codec coverage ([`9dcc033`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9dcc033e53a6508db31779b47b523ec67f4c5359))

</details>

## [4.115.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.115.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`6c65167`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c65167bbba1e54455bc672e33e04d40140ea5b6))
- update CHANGELOG.md [skip ci] ([`5ccadff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ccadff7e2fa28bcfa27547494989f497e723871))

### Changes

- Bump version: 4.114.0 → 4.115.0 ([`85a3560`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/85a3560a87d715caa8090559f339b41f156ef2ee))

### Features

- QUIC frame codec (RFC 9000 sec 19) ([`b83d798`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b83d79816b5530306ca335be58b3d3f55b10d31b))
- QUIC packet header + packet-number codec (RFC 9000 sec 17) ([`5fecc8d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5fecc8dd357d666d9c85a7d7ecc01a6f357ebf2e))

</details>

## [4.114.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.114.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`bae4c9f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bae4c9fd0f9873f77a5d6c6b4a5dea5537006bf3))
- update CHANGELOG.md [skip ci] ([`ece2c3a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ece2c3af001ee30a46aae8c35f5ac99b78b359aa))

### Changes

- Bump version: 4.113.0 → 4.114.0 ([`51a449c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51a449c19a1adadc6ac229964de31d7cd1edc4c4))

### Features

- QPACK header compression (RFC 9204) ([`8f40d00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f40d008cca1098e280fdfa40c17db9d9aa7355f))

</details>

## [4.113.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.113.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`d63bce1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d63bce11de8dada704178fdde846b879ae32a949))

### Changes

- Bump version: 4.112.0 → 4.113.0 ([`db2b2e6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/db2b2e6a2568b33d695e9fcba668cf69d65ebef0))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`d85f5d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d85f5d7b3d9ecec8648e7f44f3fc22166520ec73))

### Features

- HTTP/3 framing (RFC 9114 sec 7) ([`43863c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/43863c60fdfbf1b8f451a89003e56a4ca029d2bd))

</details>

## [4.112.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.112.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`ac5c830`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac5c8303f2bdd192b10e91626bf5b30629ba7282))

### Changes

- Bump version: 4.111.0 → 4.112.0 ([`ab136e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ab136e9b2a54a5015efc59975865b6664e3dbca7))

### Documentation

- update ESP32 build footprints [skip ci] ([`f8bdfe8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8bdfe89dc75f551e1e81d449e17fff4346b0e92))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`4a40353`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a4035353923d24f9dfb2a15a7ac8e5e2bcb48f3))

### Features

- QUIC variable-length integer codec (RFC 9000 sec 16) ([`f815310`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f81531068ee5790bc59392fabec8ced91f891869))

</details>

## [4.111.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.111.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`0af43d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0af43d7f7b440ef7dc7e58dce18d3270a72143a5))

### Changes

- Bump version: 4.110.0 → 4.111.0 ([`924fbed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/924fbeda4ef8efa5c3a5800d833304d1a636a1d3))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`6890a2f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6890a2f2939f627006d6cb3dd0249c7cd7056ccc))

### Features

- live-server wiring - HTTP/2 goes end-to-end (PSRAM-gated) ([`967e5ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/967e5edad85d249d9be9a3abb14d2a61993f3f93))

</details>

## [4.110.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.110.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`cb7a1fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cb7a1fb072b7a0d21118b386e56b66c069111542))
- update CHANGELOG.md [skip ci] ([`1dccd44`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1dccd44d34088066a200a90809a55fbd82b6c439))

### Changes

- Bump version: 4.109.0 → 4.110.0 ([`62e467f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/62e467f3736684ffbb23b631c48238a60b5e2bf3))

### Features

- connection + stream engine (RFC 9113) ([`384a628`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/384a6281bc3902342bd152af4b40aa46dc84adb1))

</details>

## [4.109.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.109.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`2dd8c77`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2dd8c77d4772173f7fb3f4533268fafb707cb179))

### Changes

- Bump version: 4.108.0 → 4.109.0 ([`129a675`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/129a675eb8318385ba5e292914d0ba1f98e9c616))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`d60df34`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d60df34ec2c3181a4e43957da5dec5c7694eb834))

### Features

- binary frame layer (RFC 9113) ([`528b7b7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/528b7b73388b27c7f106d955a3594c8997d89b65))

</details>

## [4.108.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.108.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`d9db569`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d9db5697e6fcceeb4835b1a91a2b384050bb83e6))

### Changes

- Bump version: 4.107.0 → 4.108.0 ([`93ba826`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93ba826bac8684fd1a7d0366770f995028fa1a99))

### Documentation

- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`02bb18e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/02bb18e6e4f06d39f260b45309a63e272c2f7c11))

### Features

- HPACK header compression codec (RFC 7541) ([`febf6cc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/febf6ccc8600300f9a6936812a3f2a482dea7e24))

</details>

## [4.107.0] - 2026-07-05

<details>
<summary><b>Show Changelog for version 4.107.0 - 2026-07-05</b></summary>

### CI / Build

- update test report [skip ci] ([`4a61551`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a615512ec770d2818aa65c5df7cf67bd3609ed0))

### Changes

- Bump version: 4.106.0 → 4.107.0 ([`67106db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/67106db707544ae5336c373808512e9f86c9367e))

### Documentation

- update ESP32 build footprints [skip ci] ([`083bd93`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/083bd93793939d9a20b572a2bbec7e9d4bd4506e))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`e42d70c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e42d70c935f9d99b967aca98c0ce2bf9c0f0e014))

### Features

- configurable shared I2C bus pins for Ethernet coexistence ([`604334c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/604334c7e2bfe1f0e21ab500cbdd4c4f1ce395ce))

</details>

## [4.106.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.106.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`d1f2798`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1f27981e8938e1da7bb75015c651cc4d33a513b))

### Changes

- Bump version: 4.105.0 → 4.106.0 ([`2203d7b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2203d7bbdbc898d98bc274ebf6dc429f2c70c756))

### Documentation

- update ESP32 build footprints [skip ci] ([`ed90e1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed90e1f476b8cd6ad1bc2fc209ab667708ab5328))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`8848ada`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8848adad80433e1b40eae4173bb7b6e72eb3a24b))

### Features

- TI INA219 current / power monitor codec ([`1d0b44d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d0b44df87e9b7742f4e3f565bc18c53baa91cf7))

</details>

## [4.105.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.105.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`5ccdb96`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ccdb96aa129a993bc9eb94d86eae2d665811221))

### Changes

- Bump version: 4.104.0 → 4.105.0 ([`6d260a6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d260a6127a13a252e4b81e8b1067c9026ff8c6d))

### Documentation

- update ESP32 build footprints [skip ci] ([`8aff306`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8aff306d3901be7f7618d1fe0760c4c3d2e83632))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`0b313f3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b313f34fd13aee6b7cf4ffb8c4c46e73794deaf))

### Features

- TI ADS1115 16-bit ADC codec ([`87f6b4c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/87f6b4c30894aa9530f444d544cea1b7fe07ddb7))

</details>

## [4.104.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.104.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`e96f355`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e96f355efeaa9c41448446f0857e3e29f162defa))

### Changes

- Bump version: 4.103.0 → 4.104.0 ([`0d4d3a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d4d3a777b6345724b935bd17fc0d72a27515223))

### Documentation

- update ESP32 build footprints [skip ci] ([`c2fccc5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c2fccc5983ae137e78baedd7cdcfa581b1f239d2))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`31f7a45`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/31f7a4594394c1dcb21516663cee37862af5b5da))

### Features

- NXP PCA9685 16-channel PWM / servo driver codec ([`7efbf02`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7efbf02c000c5ae1e3c20a0fd08aded08e9bfa86))

</details>

## [4.103.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.103.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`15f0fb9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/15f0fb96f18d31c66b01fff457786a4de530ecda))

### Changes

- Bump version: 4.102.0 → 4.103.0 ([`88b20de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/88b20de17fe690baffa1bd35468cd4964622f2a1))

### Documentation

- update ESP32 build footprints [skip ci] ([`ae6ffc4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae6ffc4b174dafeeca62cb346a55287a7542b25b))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`6791cf7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6791cf7fa04f9cefbe8751be8feba0bfd77f63c4))

### Features

- Sensirion SHT3x temperature/humidity sensor codec ([`3eede60`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3eede608b09394335bfb3c055e01bf2f407a73fa))

</details>

## [4.102.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.102.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`09482b8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/09482b855d97d801b622f1be0590110b10a3146f))

### Changes

- Bump version: 4.101.0 → 4.102.0 ([`8f97ac3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f97ac38b811587c63971cec56196c70ba1b0809))

### Documentation

- update ESP32 build footprints [skip ci] ([`20b6410`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/20b64109a618f24e29858ef5ba28c2b75668e5a1))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`5da336d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5da336d6a4b8de514de67cbe8b94e4657e6ffeb6))

### Features

- NXP MPR121 capacitive-touch controller codec ([`0fdf99e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0fdf99e001a8eaa0c0d5b57f744b723a0cf1ea63))

</details>

## [4.101.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.101.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`d9f1379`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d9f1379b6b661d953ee2c8c3b0369967f78a66bc))

### Changes

- Bump version: 4.100.0 → 4.101.0 ([`4cdf74c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4cdf74cd773b67108c3f9a98eb53299fdfdba771))

### Documentation

- update ESP32 build footprints [skip ci] ([`0fa2e80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0fa2e80b6cac7985112831f5bcbc53e4ec8fc7f9))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`7bb4619`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7bb46198fbe2cbcddd3400cc5b93ffd5d3f6c228))

### Features

- HLK-LD2410 mmWave presence/motion radar codec ([`6c1a578`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c1a578572f596daf0a6d57a71a307a59cf641ea))

</details>

## [4.100.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.100.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`d8304ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d8304adea52d87ed056c3d792643b257bc5f04d1))

### Changes

- Bump version: 4.99.0 → 4.100.0 ([`ad1b17c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ad1b17cbdd37a11c6304f2a81d5454e2ad2d1774))

### Documentation

- update ESP32 build footprints [skip ci] ([`cdf30d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cdf30d0ba2a4b65e64c8bcfd62de66a9785bf277))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`2fb1944`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2fb1944045b78e64ff5c36859df309a60e03f0f0))

### Features

- DS1307/DS3231 battery-backed RTC time source ([`bf7a74f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf7a74fee439f0ee827fc651240ca957d2ec4298))

</details>

## [4.99.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.99.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`a3e1b32`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3e1b3231e86eb8b05ec084ec215ad202927a7f5))
- update test report [skip ci] ([`dda09b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dda09b3ce19d3a8cc87eb0fab81ebabe0b0b3f77))

### Changes

- Bump version: 4.98.0 → 4.99.0 ([`1ec42f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ec42f82013218e6cfb2abee6051c11a1580d378))
- clang-format the 59.StatsdMetrics sketch (trailing-comment alignment) ([`9e62f01`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e62f011204a5d74a1f6c6397dee20cb7df76f85))

### Documentation

- update ESP32 build footprints [skip ci] ([`d7d4174`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d7d41745ec11a3f06000092ce2c719a2081854ae))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`74a325b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/74a325b1b8c9def03261d5b886594a4c04a4c636))

### Features

- authoritative DNS server (name->IP A records) on UDP/53 ([`12a176f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12a176fbc2ce88cede62eebbaa672507e4e0511f))

</details>

## [4.98.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.98.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`f169458`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f169458053c35c988ef199f1f83a31fd01e31b25))

### Changes

- Bump version: 4.97.0 → 4.98.0 ([`7d7b2d8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7d7b2d8a3def46261dfb62f84fd3815c4ebd277c))

### Documentation

- update ESP32 build footprints [skip ci] ([`cc867db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cc867db7f205fcd80c6ff5a8367cc26824cc12d6))
- regenerate feature tables + configurator + build_opt.h + example index [skip ci] ([`260c71d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/260c71d5a07a95b2d5f1f14f700fe31087edbd8d))

### Features

- StatsD metrics client (push counters/gauges/timings/sets) ([`754e2bd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/754e2bdef95672c706c3f028526859e221226a7f))

</details>

## [4.97.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.97.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`8c7e939`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8c7e939d211b88017a26d70b4af3bcbbc59acf92))
- update CHANGELOG.md [skip ci] ([`43807f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/43807f229ef5526d03849b8c319ae9be3c5feeac))
- update test report [skip ci] ([`28c3fd0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/28c3fd0ab1507bd11d92343889bc9379e3544077))
- update CHANGELOG.md [skip ci] ([`72aa50e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72aa50e0bc10b10b1abedca6f83c5f45b4560466))
- update test report [skip ci] ([`a5e42be`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a5e42be3b163b7565354a99fcb42945aaf2e3fc2))
- update CHANGELOG.md [skip ci] ([`8983609`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8983609d4968d4f3df8f23cc227976d5968be52f))
- add STARTTLS to the spellcheck dictionary ([`d898c05`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d898c05bc8424ab0e64d3f918b3910ccb22bde76))
- update test report [skip ci] ([`49467bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/49467bbaa253bed8ba49a49a5deee28ab27d753a))

### Changes

- Bump version: 4.96.0 → 4.97.0 ([`dcfdc48`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dcfdc48ae4a643a28e58f2421651946309bc8a51))

### Documentation

- style the OSI feature tables - full width, centered, collapsed, themed ([`1dd6580`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1dd658018c543ec4a04965d1ed25fe31bead23ac))
- generate EXAMPLES.md from examples/; group README feature tables by OSI layer ([`d677096`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d67709614928196e14d445a8adbd7153ea794d08))
- update ESP32 build footprints [skip ci] ([`1c664a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c664a360c617ebfa88171dc38102063da1b1f50))
- regenerate feature tables + configurator + build_opt.h [skip ci] ([`0c9ea82`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0c9ea82221a5f07daece2fcd41af5611e4c9d5e3))

### Features

- NTP/SNTP time server (RFC 5905 server mode) on UDP/123 ([`be03964`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be0396404be9e9a513e7cc48872744c170853ef0))

</details>

## [4.96.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.96.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`6770ace`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6770ace6684a010a8a9bd3d5fa8d34942f2530b5))
- update CHANGELOG.md [skip ci] ([`028d552`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/028d55286843359931343be7b12afc7d8382d44e))

### Changes

- Bump version: 4.95.3 → 4.96.0 ([`92a405b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92a405b6d6e660776d317fdae4110e619679b352))

### Features

- outbound SMTP client (RFC 5321) for device email alerts ([`a863b13`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a863b1378102d45d7ac8b1e5c29fe231f4638e27))

</details>

## [4.95.3] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.95.3 - 2026-07-04</b></summary>

### Bug Fixes

- remediate real CodeQL/SonarCloud findings + docs audit ([`33804c7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33804c7ec27ff3c24735610493b40a639641ce5b))

### CI / Build

- update test report [skip ci] ([`24446ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24446ec42afe884c4e79e720779e505d54776547))
- update CHANGELOG.md [skip ci] ([`9caba8a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9caba8a31f76e798891b47a92d83a48cfa2fe196))

### Changes

- Bump version: 4.95.2 → 4.95.3 ([`da97daf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/da97daf3309dd240aff4ad38df8ec78d26bc231e))

### Documentation

- update ESP32 build footprints [skip ci] ([`7c9b309`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7c9b3094ed3898f96bdb711a3c30c511726f0c6b))

</details>

## [4.95.2] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.95.2 - 2026-07-04</b></summary>

### Bug Fixes

- marshal UDP (det_udp_*) onto tcpip_thread ([`6718a5b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6718a5bc7646d64e6cf69385612eadf4a684a5df))

### CI / Build

- update test report [skip ci] ([`a6ccd1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a6ccd1cc28d522a64f6f78ce81f490d47bb4a0bd))
- update CHANGELOG.md [skip ci] ([`9eabc24`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9eabc245de9fe02f16e951460e55fcfc1b78b765))

### Changes

- Bump version: 4.95.1 → 4.95.2 ([`4f45756`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f4575671be925b0ac421be322f1f0ec5a18efc3))

### Documentation

- update ESP32 build footprints [skip ci] ([`eae75fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eae75fb47a3336c20d7a959038ef9ba906f06d41))

</details>

## [4.95.1] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.95.1 - 2026-07-04</b></summary>

### Bug Fixes

- marshal primary listener bring-up onto tcpip_thread ([`554d338`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/554d338e15a72a9fd9932632208f6d26c5fb04c3))

### CI / Build

- update test report [skip ci] ([`7d437a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7d437a290ae779740ed000b6bc102b8b27299b1c))
- update CHANGELOG.md [skip ci] ([`b70a5f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b70a5f7c200512a7346f697c964bdb51a24ddc77))

### Changes

- Bump version: 4.95.0 → 4.95.1 ([`a3abf3d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3abf3daa3f1f852941cdfc829d5ed97ee82c1b4))

### Documentation

- sync shipped status - SSH ECC (v4.94.0) + PSRAM TLS arena (v4.95.0) ([`ddc6913`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ddc6913d7a28d3a4742de37cd9b71a5137836ecd))

</details>

## [4.95.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.95.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`84ae7ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84ae7baea541dd4d510ddc05982117a4d38874ab))
- update CHANGELOG.md [skip ci] ([`5672a2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5672a2b5033ded62d452417a3e8a5e04c05b4233))
- update test report [skip ci] ([`6be3932`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6be39325522ea151afc410820fe6db8a6cb5eaf3))
- update CHANGELOG.md [skip ci] ([`aceb09c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aceb09c44b904769e4f3a19e9a84394e834c9011))
- update test report [skip ci] ([`1ee35af`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ee35af1bec95a5f9ea03b153fff0de5551059de))
- update CHANGELOG.md [skip ci] ([`ef4751d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef4751d34ff03db8b40df6378584a4739ddb886e))
- update test report [skip ci] ([`6264984`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6264984fc8eedcafdda3e2f04455c6f62913aa18))
- update CHANGELOG.md [skip ci] ([`46c1eb9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46c1eb9c4b43a249a202567d3ae125a5ca95c576))
- keep shell scripts LF via .gitattributes (shebang safety) ([`66f1792`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/66f17921d7e4d6f801d42e5ae99d25140cd7555f))
- update test report [skip ci] ([`fff11df`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fff11df8bbcf59ef61d9dd0e36973b6a3cb68eea))
- update CHANGELOG.md [skip ci] ([`66c2956`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/66c29562d1e04312e507ba2923f8e3068058e559))
- add ECDH, hostkey to cspell dictionary ([`1992202`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/19922024c7e83f6e7863d1107d333a1506576799))

### Changes

- Bump version: 4.94.0 → 4.95.0 ([`4ef8e4f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ef8e4fc54e0e403e41c1d743c91249fcaded59f))

### Documentation

- frame the PSRAM flash-cache caveat as a whole-build choice (no per-slot split) ([`ba93267`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba932676fbf02575ee215af02d8561cc783078df))
- note PSRAM-arena rebuild is HW-verified on an S3 N16R8 ([`06ded53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06ded53324e0dc9256c9cd576e2ee047f25930bf))
- the recipe that actually works - Docker image + low parallelism (HW-verified) ([`98c0af9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/98c0af923d3217e22436a0e204c0fbf48c055ae3))
- fix lib-builder ref + prereqs from real runs; add newbie prebuilt-core path ([`aab5dc6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aab5dc64896089526834682493a5457cf18b1580))
- update ESP32 build footprints [skip ci] ([`af4906c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/af4906ca96c805a6d1e94ebaeebb03200195a2f8))
- regenerate feature tables + configurator + build_opt.h [skip ci] ([`f2a627c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2a627c8f489f3cbf8cdd431572d980ae9d7f0d4))

### Features

- det_arena scratch alignment (up to 16B) + document the primitive ([`610d94d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/610d94d75ed23e0fd7bfe9ad649de135bd7f9afd))
- det_arena multi-region set (DRAM base + PSRAM extension) ([`3b18f01`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b18f01982c70b7d4f1b12caedd306879b27c20a))
- unified double-ended server arena (det_arena) core ([`1486420`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1486420ce1452b20197855988960238bb7608793))
- fail-closed when DETWS_TLS_ARENA_IN_PSRAM lacks the framework flag + core-rebuild tooling ([`54c4d8e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/54c4d8eabd9914e42dfc523338940c7048c5ef89))

</details>

## [4.94.0] - 2026-07-04

<details>
<summary><b>Show Changelog for version 4.94.0 - 2026-07-04</b></summary>

### CI / Build

- update test report [skip ci] ([`db4c0f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/db4c0f465ba725ac33cd7320289c346537b1247a))
- update CHANGELOG.md [skip ci] ([`d4dc403`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d4dc40363fff82db2da5215466faf33606474f4a))
- update test report [skip ci] ([`c426301`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c426301280c81bd0374dc441f175a13b4011ab97))
- update CHANGELOG.md [skip ci] ([`6ccc1bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ccc1bb46f8f8281c0dbf2270e29682729c3e460))
- bump prettier from 3.9.1 to 3.9.4 ([`9dd2486`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9dd2486ae39d4f52abf2efdf48dec7780493fc87))
- update test report [skip ci] ([`652c507`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/652c507be3911fbbeee05c93b7b3ee4861ea9898))

### Changes

- Bump version: 4.93.0 → 4.94.0 ([`26c34bd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/26c34bdd73dcdc52fb00a499d5a31bfdc3b5f2ed))
- Merge pull request #13 from dstroy0/dependabot/npm_and_yarn/prettier-3.9.4 ([`aac9fa4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aac9fa4be01829f612dd1346ae972751f58273be))

### Documentation

- update ESP32 build footprints [skip ci] ([`4988aaf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4988aaf0c99813123b877c76502f9ebd9897ea0e))
- regenerate feature tables + configurator + build_opt.h [skip ci] ([`b66e6c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b66e6c5214bae6cb9736ec528ea3601c471657c6))

### Features

- crypto-agnostic KEX - curve25519-sha256 + ssh-ed25519, HW-accelerated ([`9e46dc9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e46dc914e7514d7abfdfcd0c2221b4c328b4025))
- add SHA-512 + X25519 + Ed25519 crypto primitives (RFC KAT-verified) ([`f6b63a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f6b63a7e16b015f3226da7d0477ce9b8fefd7ade))

</details>

## [4.93.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.93.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`4f9a600`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f9a600cf83f21b30cf74383430396f179967ecd))
- update CHANGELOG.md [skip ci] ([`71e0a31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71e0a31635d1667964886ea8374a2882b24697af))

### Changes

- Bump version: 4.92.0 → 4.93.0 ([`a8ea06a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8ea06a83b26da817ed950be1a6a3288cf58cd4f))

### Documentation

- update ESP32 build footprints [skip ci] ([`c1c75e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c1c75e862b6a7b6dca176fd8400a2e424850932e))

### Features

- remote port forwarding (ssh -R, forwarded-tcpip) - HW verified ([`3c89dd8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c89dd85ffb96d99921557b098867da52a535ad7))

</details>

## [4.92.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.92.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`4f886fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f886fb7fd0be19589cd0619d70017e7fb86b80c))
- update CHANGELOG.md [skip ci] ([`985c082`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/985c0828f20ff09a7ea9e63d3998eea4e2683b5b))

### Changes

- Bump version: 4.91.0 → 4.92.0 ([`739bd81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/739bd8122abfe3916ab66cf477c8dcc23719051c))

### Documentation

- update ESP32 build footprints [skip ci] ([`4b48dee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b48deea60d3cab24b96081cd554e34e786b63bb))

### Features

- RFC 4254 global-request handling + tcpip-forward (ssh -R) seam ([`60292e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/60292e0ed6a80e563fa59423ffb1ddb7c6cea2ba))

</details>

## [4.91.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.91.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`fa30779`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa307790a8ad0cd5b1d91a9eab2b62964b92c47a))
- update CHANGELOG.md [skip ci] ([`b8e349a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b8e349a23ddc89f5ff9fb52d2b6d3d08eb3591be))

### Changes

- Bump version: 4.90.0 → 4.91.0 ([`622a022`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/622a022788bc1de5d12f916d04a299c364bdb94a))

### Documentation

- update ESP32 build footprints [skip ci] ([`5f142d3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5f142d3f76eeec5c2dd3751901e0239de9c80fe2))

### Features

- enforce exp / nbf time claims when a clock is available ([`7161cae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7161caed1c6644975de4d05be6cc0f9a0796f5d5))

</details>

## [4.90.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.90.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`f82dedf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f82dedf96f6a07d9bf0557add35bd70c813530c1))
- update CHANGELOG.md [skip ci] ([`dd2c086`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dd2c0864e73828bf1c717a5a5f1b1cecbffe4d65))
- update test report [skip ci] ([`bb240e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb240e1ff4fe4819486da0066dcb88bb8ef2ed39))
- update CHANGELOG.md [skip ci] ([`140a022`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/140a022d6409884d92d0a45a4b0211cc3c7fdd4d))
- add collidable/sockaddr/detip to cspell dictionary ([`bcc0fbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bcc0fbec33c0fdd6842a42baa7a6e7d087695862))
- update test report [skip ci] ([`822d5ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/822d5cec1db0ee99a7fc5c1a5780d5eacb6f14d6))

### Changes

- Bump version: 4.89.0 → 4.90.0 ([`fa7633f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa7633f799a2723d85260824866e0af051008345))

### Documentation

- mark IPv6 phase 2 done (full-address DetIp keying, v4.89.0) ([`84d94af`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84d94af8569e4138bb4e80b499914a1e74da56d0))
- update ESP32 build footprints [skip ci] ([`5fb35ae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5fb35aeb77df354236e48dc068577e05674f9544))
- regenerate feature tables + configurator + build_opt.h [skip ci] ([`d3fc5b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3fc5b26691b635da9106fb44da9ed4cd727568f))

### Features

- recover IPv6 client addresses from Forwarded / X-Forwarded-For ([`0d835ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d835abe761cb6e1012a49ec95fd1ef41373a406))

</details>

## [4.89.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.89.0 - 2026-07-03</b></summary>

### Bug Fixes

- key IP abuse-prevention on the full address, not a 32-bit hash ([`ba4b606`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba4b60698002e4cfe1aafaa6000c177fe1a1b10a))

### CI / Build

- update test report [skip ci] ([`4871751`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4871751fa337c87367a53fbe9f333722cf6896c7))
- update CHANGELOG.md [skip ci] ([`1671a90`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1671a90dea70ef7b1860d0c3278c5882647041fd))

### Changes

- Bump version: 4.88.0 → 4.89.0 ([`b81de26`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b81de261fbaff096b129d9bd7a424eed84537612))

### Documentation

- update ESP32 build footprints [skip ci] ([`9d09c7a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9d09c7a18a4ecc5952df1b93ac3b971acee67d7c))

</details>

## [4.88.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.88.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`16aad67`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/16aad673c827d9ac4b7d58071464a67e4ae8796c))
- update CHANGELOG.md [skip ci] ([`e67a5b5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e67a5b58ac22ccddffb12b5126db91d6bd7e40dc))

### Changes

- Bump version: 4.87.0 → 4.88.0 ([`bde177e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bde177e326cd43b8515a41b16f481aef5c6eac93))

### Documentation

- update ESP32 build footprints [skip ci] ([`c7da665`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7da665f45bc1284ddaf660eeeab18107abd58ac))

### Features

- per-IP accept throttle is now IPv6-safe (family-stable key) - IPv6 phase 2 ([`719a827`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/719a827f264011c3e070c0d41e4691f81fd79523))

</details>

## [4.87.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.87.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`fc4cd60`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fc4cd6064c1a6ce9ad98beb3e6c0fa1ef584707a))
- update CHANGELOG.md [skip ci] ([`f531ae9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f531ae993d780b50516d6efde4d513360c29b899))

### Changes

- Bump version: 4.86.1 → 4.87.0 ([`49a94f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/49a94f431227a4ed7d4b8c35f55b2815bebe2060))

### Documentation

- update ESP32 build footprints [skip ci] ([`eccb541`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eccb5415f18d245374560c717d619289b5464860))

### Features

- auth lockout is now IPv6-safe (family-stable per-peer key) - IPv6 phase 2 ([`e281ec6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e281ec65f973c77eb16566d6a6990602e5dc61d4))

</details>

## [4.86.1] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.86.1 - 2026-07-03</b></summary>

### Bug Fixes

- clang-format test_promisc.cpp (missed after the det_pcap rename) ([`d9d60ca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d9d60ca7d03ad01aa226f0a1e2dbcfbc03832a2b))

### CI / Build

- update test report [skip ci] ([`ebf94b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ebf94b6e085100358df61ebfdb51160a0f4894cc))
- update CHANGELOG.md [skip ci] ([`ee8a0dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee8a0dcbaf6c2c8f9ce9c36c3106ea4b2f78e650))

### Changes

- Bump version: 4.86.0 → 4.86.1 ([`19b5368`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/19b536836cd52b2d7384054b669bbf8cefa258b8))

</details>

## [4.86.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.86.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`0f3b70e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0f3b70e1a15ffa7b25bd13ed3f4f8607b42f63a4))
- update CHANGELOG.md [skip ci] ([`4c49a07`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c49a0735863fc95888037e018ca4ffab2f3db5b))

### Changes

- Bump version: 4.85.0 → 4.86.0 ([`9d65fd3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9d65fd34cc97dfe24476e3f851ca19ec03727d9d))

### Documentation

- update ESP32 build footprints [skip ci] ([`e8c4c6b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e8c4c6bac3b339101c9444d7f9c065973d3f774f))

### Refactor

- cut cognitive complexity + nested ternaries (SonarCloud, no-idiom-clash subset) ([`f77af21`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f77af21c21c45829435c6d7f5ce7d32300e12388))
- split into single-purpose helpers; + DetIp peer-address API (IPv6 phase 2) ([`a6d4ef7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a6d4ef7d3ca57242dacbe111d5f9ebcb06975cf2))

</details>

## [4.85.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.85.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`9e15d45`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e15d451db7780f40843efd914778ecd84b1601b))
- update CHANGELOG.md [skip ci] ([`b189631`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b189631c30f52f5006595a3d2356ed52b932f1b2))

### Changes

- Bump version: 4.84.0 → 4.85.0 ([`0b76e31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b76e31954740ef5196891da1b7d21a76feda699))

### Documentation

- update ESP32 build footprints [skip ci] ([`05095e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/05095e8315d8e2e3b96312f4dff4cd3cedb7c24f))

### Features

- CAN listen-only capture -> forward (services/bus_capture) + shared PCAP owner ([`d2641c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2641c0e416d7a5c02fe5cf39e092c1df8ab5b31))

</details>

## [4.84.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.84.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`2986ba5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2986ba559288500e7050c75b76c14dda604452f6))
- update CHANGELOG.md [skip ci] ([`c6c2be6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c6c2be6f5830c58fd65b98b19069143936cca272))

### Changes

- Bump version: 4.83.0 → 4.84.0 ([`273edb6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/273edb6f2d7f4fa69a3569acfcb34366e54be522))

### Documentation

- update ESP32 build footprints [skip ci] ([`4a68241`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a6824170664cc3938d96e1163dd8e5c5f461fad))

### Features

- Wi-Fi promiscuous capture + forward to Ethernet (services/promisc) ([`8548dd6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8548dd63eb3fa6956dddba97ba0ea6e04e8c5811))

</details>

## [4.83.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.83.0 - 2026-07-03</b></summary>

### CI / Build

- update test report [skip ci] ([`12088bd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12088bdeb06980e302c0302346d7f1d3f3283ddf))
- update CHANGELOG.md [skip ci] ([`cde5f07`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cde5f0737837fdd517114f5337e1cdbf3bf7b8fd))

### Changes

- Bump version: 4.82.1 → 4.83.0 ([`e9f0b22`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e9f0b22de2e3f0429f0ee5ae1cf495eb2f28ff26))

### Documentation

- update ESP32 build footprints [skip ci] ([`cf99aba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf99aba425a9177c9e7ff80a6e90e185e43eca42))

### Features

- IPv6 dual-stack phase 1 - DetIp address core + netif bring-up ([`7a8406f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a8406f8e0b1de1729bfac5a6c6d3d812e5d2877))

</details>

## [4.82.1] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.82.1 - 2026-07-03</b></summary>

### Bug Fixes

- keep clang-format green for the Arduino build_opt.h work ([`3fc3a35`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3fc3a35c2e5e173058b139519c833159a02f945a))

### CI / Build

- update test report [skip ci] ([`606a9a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/606a9a13e9b022381d320496118cc472ae1b3789))
- update CHANGELOG.md [skip ci] ([`935fb31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/935fb3160e9c89d4f3a9e4eea7634005aaa83a7e))

### Changes

- Bump version: 4.82.0 → 4.82.1 ([`b29df05`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b29df05b95666190f9d1ab7c7caa64ee4ce6dd0b))

</details>

## [4.82.0] - 2026-07-03

<details>
<summary><b>Show Changelog for version 4.82.0 - 2026-07-03</b></summary>

### CI / Build

- add Arduino Build workflow (esp32 3.x) + fix 06.PreemptQueue timer API ([`4e26b81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e26b81301415af0e3fce0af8bfb62a32bac9e59))
- update test report [skip ci] ([`f112264`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f1122641095591753000b83e476f14872b1e0c57))
- update CHANGELOG.md [skip ci] ([`1b9563f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1b9563f72457e7207803dd49c1fca252430718df))

### Changes

- Bump version: 4.81.0 → 4.82.0 ([`058db0e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/058db0e9725364267a9ab83cae64f41761704320))

### Features

- make the library build unmodified in the Arduino IDE (esp32 3.x) ([`bc53dab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bc53dab0bedc100b236001de67b47e0691c4c2bc))

</details>

## [4.81.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.81.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`19a3486`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/19a3486a3309a36d733b52846cf00b2b7c4411c6))
- update CHANGELOG.md [skip ci] ([`86184e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/86184e1cc9aee0044954c227b852e297c94ccd17))

### Changes

- Bump version: 4.80.0 → 4.81.0 ([`d204bb0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d204bb0b48f4014708b672dac25cdc667fc5b5f2))

### Features

- interactive build configurator generated from the config header ([`70a6698`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/70a66988beecfc58b0c4d4f537151422fd126d4a))

</details>

## [4.80.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.80.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`995cd69`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/995cd69bbb3567ff997b0587eb9d2b601d77f47b))
- update CHANGELOG.md [skip ci] ([`c8ecc99`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c8ecc99e9e761a986a0c083b22a54a3f78002c22))

### Changes

- Bump version: 4.79.0 → 4.80.0 ([`6f5067a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6f5067a04e7ffba1d7254e41dc198776ad503d24))

### Refactor

- centralize all tuning knobs in DetWebServerConfig.h ([`3852708`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38527081ab839aeeb2a72c4c6dc26cffa343fa43))

</details>

## [4.79.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.79.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`7732c5e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7732c5e2bd69cac4f3b82cf9c05c30425425aba3))
- update CHANGELOG.md [skip ci] ([`d0460b7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0460b7f759377d2bb1787c476ce9fd1cd645153))

### Changes

- Bump version: 4.78.0 → 4.79.0 ([`a41cf5a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a41cf5a9a64e5c5cc57ccba219e0fe061a0c3b91))

### Documentation

- update ESP32 build footprints [skip ci] ([`ac5dc8e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac5dc8e9aac801ec9fdd69b24a6acedd6c337663))

### Features

- spinel command layer on the HDLC framing ([`97d3cd1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/97d3cd178a2ca295bfb6106e78ef00acbf845d3f))

</details>

## [4.78.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.78.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`26b24d2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/26b24d20520c1923e5339421bddf08079f50fea2))
- update CHANGELOG.md [skip ci] ([`c14817c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c14817c19b033dfb7611a0e615a04e53e91a7eb5))
- allow Rohm / devboard (Wi-SUN roadmap note) ([`0576b54`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0576b54f4fe632ff8c9cd9faa8e3fb02b5621f2e))
- update CHANGELOG.md [skip ci] ([`9dd0242`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9dd02424da542579abf566ec4201a4b3a1cacd9f))
- update test report [skip ci] ([`fb482cd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb482cd78f1ec764019aec89e0c21b29fb8d0f1a))
- update CHANGELOG.md [skip ci] ([`a517562`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a5175626111f7eccd4316fc3ee43004a38550f0e))

### Changes

- Bump version: 4.77.0 → 4.78.0 ([`874979e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/874979e4f913fd59776764faff018bf8b9fa94f1))

### Documentation

- reclassify Wi-SUN as a connector, not a radio-module driver ([`a50db36`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a50db3675b15b4a493ec10a7d78c0211f9e3c44e))
- update ESP32 build footprints [skip ci] ([`999f87b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/999f87b942c1656ea79479bd99a37d326e3aacd0))

### Features

- wired Ethernet PHY bring-up (DETWS_ENABLE_ETHERNET) ([`7e14835`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e148353d0d80a0e3caf12eb49da2cee28afa8a1))

</details>

## [4.77.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.77.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`c7e41d3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7e41d36c42c52ad282e30bf3c967ad3d80f803d))
- update CHANGELOG.md [skip ci] ([`977d70c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/977d70c38cf2efd1f9b464ca5b26fb35f5b69dd8))

### Changes

- Bump version: 4.76.0 → 4.77.0 ([`ccbab68`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ccbab680e5488724670b8d5a4622a51d266a7c9c))

### Documentation

- update ESP32 build footprints [skip ci] ([`6b4533a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6b4533a4a6b13be87ebe8a5703aaeebcc0622ae3))

### Features

- Thread spinel / HDLC-lite framing codec - a gateway plugin ([`77bee5b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/77bee5bc8ad69c935ea761808ee68afcdb2b4ffb))

</details>

## [4.76.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.76.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`829a0a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/829a0a374db186fa0b8f604107c86468f8d8614f))
- update CHANGELOG.md [skip ci] ([`fb41cba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb41cbaa18b91ce037ceeded6737a6752da9c28d))

### Changes

- Bump version: 4.75.0 → 4.76.0 ([`16306ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/16306ad1bbcc14e81029d7a4056296465279562a))

### Documentation

- update ESP32 build footprints [skip ci] ([`441ace1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/441ace1f76c8225953249274e87f476027e316e3))

### Features

- Zigbee EZSP/ASH framing codec - a gateway plugin ([`904d6bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/904d6bf3e6d1f48d4e3bf6c9bccb5070871ca108))

</details>

## [4.75.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.75.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`41d5c2f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/41d5c2ff10b327659ce1c554d93eac68e5b5db6c))
- update CHANGELOG.md [skip ci] ([`76768a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76768a08f2509aa1d56b5c5ddc572d33f2db31cf))

### Changes

- Bump version: 4.74.0 → 4.75.0 ([`84145ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84145ce51d5fcdc797b3ddd18a23ccb831775a8f))

### Documentation

- update ESP32 build footprints [skip ci] ([`edd581a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/edd581a6937037cc9601343f865b69a67482cad9))

### Features

- Z-Wave Serial API frame codec - a gateway plugin ([`954a874`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/954a87467a314da004377b99516ff0e78ae969c9))

</details>

## [4.74.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.74.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`a3d8368`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3d8368311da179f1ca96800a5d3ae95d6e6faaa))
- update CHANGELOG.md [skip ci] ([`c09125b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c09125b0c76bdc12f06a4c549c4776f571a2d422))

### Changes

- Bump version: 4.73.0 → 4.74.0 ([`2810160`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/28101606f4de37f75c154f3b200454c64f1fe1b2))

### Documentation

- update ESP32 build footprints [skip ci] ([`2f4e930`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2f4e9303e7330019b42fb6fa74bec4e619bfaa8f))

### Features

- Sigfox modem AT-command codec - tiny low-power uplinks ([`d512d38`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d512d3836ad00c9851740ff82684f4502d85d83c))

</details>

## [4.73.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.73.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`d8d5364`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d8d53642f5885f9f3b2d0ed324e2c0be4342abc6))
- update CHANGELOG.md [skip ci] ([`194e0ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/194e0ed421b38c8254d7532743e996f49ee8cc57))

### Changes

- Bump version: 4.72.0 → 4.73.0 ([`918f6f0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/918f6f000dccb1d62e71f19fe4f490bb52247bd9))

### Documentation

- update ESP32 build footprints [skip ci] ([`82b5580`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82b5580de18647341ad41be2310185c58a067ad5))

### Features

- PN532 NFC frame codec - a gateway plugin (tag read -> web event) ([`2ffa45c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ffa45cbeddf567d155447560df9e77888390c45))

</details>

## [4.72.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.72.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`d47d98d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d47d98d4ccac6ff8f125fb78c7aa70d8b6d33067))
- update CHANGELOG.md [skip ci] ([`db33976`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/db33976d3b94ef35ee19e11fa9fec233e84bb625))

### Changes

- Bump version: 4.71.1 → 4.72.0 ([`7825077`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/782507704f7107a7bb427c00581825a7654700ce))

### Features

- EnOcean ESP3 serial codec - a UART radio plugin for the gateway ([`48a5d48`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/48a5d480c7c491c12d43bbe5824e19b8d75e7c89))

</details>

## [4.71.1] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.71.1 - 2026-07-02</b></summary>

### Bug Fixes

- split det_gw_topic separator writes (SonarCloud S1764) ([`ffc1c54`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ffc1c54cc536d202442e3ab2024d2ae40233c932))

### CI / Build

- update test report [skip ci] ([`50e83c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50e83c04917e1e67f0212889c3396590f0fd97d6))
- update CHANGELOG.md [skip ci] ([`b18c558`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b18c558937c0cd54fa7ca9f218e128104b0cbef4))

### Changes

- Bump version: 4.71.0 → 4.71.1 ([`ede11e6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ede11e61bac9a5f2cde17964f04d5c3ad5efa048))

### Documentation

- update ESP32 build footprints [skip ci] ([`849a30a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/849a30aa28fedd6dca1d5c96076485b0373a1cb2))

</details>

## [4.71.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.71.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`d1cacee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1cacee942329c1ae51ba76ba60cf0e6b91d6bb1))
- update CHANGELOG.md [skip ci] ([`5ec1584`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ec15849e7497a43101221d29e1476c57e88b6b7))
- update test report [skip ci] ([`d7c2cbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d7c2cbe06214831349a1d082aaaa37eb45c6f347))
- update CHANGELOG.md [skip ci] ([`88747f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/88747f5a74549257c514c2ae52cc36302139aecf))
- update test report [skip ci] ([`22d7807`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/22d78078357c004e979e02b792ef818bffeea63f))
- update CHANGELOG.md [skip ci] ([`dff1a3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dff1a3f2ed36214b45d3fb3468d79e0909e4db36))

### Changes

- Bump version: 4.70.0 → 4.71.0 ([`b526fb2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b526fb2f1afbaf6b05fd4b23abd7d0a1917ca820))

### Documentation

- add a DRIVERS table + a categorized CI badge table ([`17bf39c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17bf39c008b361826030da9063bcef37c9d0cf0e))
- move LoRa to the codecs table ([`70afed4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/70afed456d874f2a3e47e51ac7fac6db9ec65746))
- update ESP32 build footprints [skip ci] ([`4989a81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4989a81f35e2396ab4b9617af7f32e0cf04bc461))

### Features

- nRF24L01+ radio driver - a second gateway radio plugin ([`927aa1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/927aa1cdbf73bca276a8c09b38fc63e83245c49f))

</details>

## [4.70.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.70.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`c669657`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c669657ff0d72b5a7413ca690989c0a9ac42e269))
- update CHANGELOG.md [skip ci] ([`4591246`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/459124627d097e8ba8322e54570bfdab5c3c6886))

### Changes

- Bump version: 4.69.0 → 4.70.0 ([`d39a7db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d39a7db3fed42b59b97ce7403d1ada7462010e36))

### Documentation

- update ESP32 build footprints [skip ci] ([`6dc3571`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6dc357187837535662ce26b1928d2216ed6ef3a7))

### Features

- LoRa codec + SX127x driver - a radio plugin for the gateway ([`8a1520b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a1520bb9fd2e84853132680bb497c0317ca8b3d))

</details>

## [4.69.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.69.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`81179e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/81179e1b6902e7a25ba86ba85d4fd5278b7ac964))
- update CHANGELOG.md [skip ci] ([`e0aab89`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e0aab89881660f87efd32f16d5942de72d27c952))

### Changes

- Bump version: 4.68.0 → 4.69.0 ([`600ef93`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/600ef933a98d7ac3914dba8f8fa9ed6dfae8cd78))

### Documentation

- update ESP32 build footprints [skip ci] ([`3b9766e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b9766e2660a60cfff57e8641bba9d85a175e9d0))

### Features

- radio gateway bridge - southbound radio to northbound stack ([`759ac6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/759ac6e31edc9c3bef06771f7a1b307487b6a9e6))

</details>

## [4.68.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.68.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`0f42d18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0f42d18aade027dd4b53dfbe133de97213bc4c05))
- update CHANGELOG.md [skip ci] ([`7d023f0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7d023f0a7d89e9bc627932a1f2bd74320987a6b4))

### Changes

- Bump version: 4.67.0 → 4.68.0 ([`dcae78b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dcae78b2ceac52d3cf4243789c623c85e9e914ed))

### Documentation

- update ESP32 build footprints [skip ci] ([`a0f8b6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0f8b6c4a07842365918b17fef1532db5b71b53c))

### Features

- ingress ACL - byte-pattern permit/deny before forwarding ([`8c6796d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8c6796db4d5895b667f6c324e4bc13e5f8f47296))

</details>

## [4.67.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.67.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`14f1adb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14f1adbe4117270484b01bda2fc2047036962828))
- update CHANGELOG.md [skip ci] ([`a17bea8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a17bea857802e9f27e61709ba7e7f23a365b1f1e))

### Changes

- Bump version: 4.66.0 → 4.67.0 ([`d7b8f4d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d7b8f4db0e53b15b42af47a59f795eb3d5a7c223))

### Documentation

- update ESP32 build footprints [skip ci] ([`4e4ce62`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e4ce624f99374eb7d4c8458eb38fc1100aecbb1))

### Features

- interface forwarding plane - DMA-driven bridge / router ([`b821463`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b821463f28b6955bc98e7dd3aa650ea57741f0e8))

</details>

## [4.66.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.66.0 - 2026-07-02</b></summary>

### CI / Build

- update test report [skip ci] ([`5403779`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5403779ee6b6ca8c21266cd05cf2f135fdeeefc8))
- update CHANGELOG.md [skip ci] ([`a03384b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a03384b3812634ef1e69ef7bb2492559e236aef8))

### Changes

- Bump version: 4.65.0 → 4.66.0 ([`0bb0f80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0bb0f80f0262794e1a020a130151cbe3cab5477e))

### Documentation

- update ESP32 build footprints [skip ci] ([`7a006c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a006c5238ac76933c23f807ae747c6bbaa3f77d))

### Features

- named lanes - internal DMA/FORWARD/DEVICE above the user lane ([`e62e6f0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e62e6f0dee7ef183e9cae7c53623373a565cfbf8))

</details>

## [4.65.0] - 2026-07-02

<details>
<summary><b>Show Changelog for version 4.65.0 - 2026-07-02</b></summary>

### Bug Fixes

- extend coverage, probably introduced some SonarQube errors, flying blind on my phone ([`0a162c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a162c530d85ed85d2cdabda49be8d0a011380e7))

### CI / Build

- update test report [skip ci] ([`6872917`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6872917a07cb4b40e0383aafafaa44c71a79abda))
- update CHANGELOG.md [skip ci] ([`23c4a7e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23c4a7ef505a4ef2d06a3259d80864ec3588f160))
- update test report [skip ci] ([`3ce252c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3ce252cfaee5abfb91a3479735251e0bc6d17176))
- update CHANGELOG.md [skip ci] ([`7178b27`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7178b27daef8fe6dc2792134dd49bd8ee9c9b7cb))
- update test report [skip ci] ([`829828e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/829828e37b7e6e10ab52c7745b8876f3b7122daa))
- update CHANGELOG.md [skip ci] ([`b43f568`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b43f568932c528c9533ab72a5a2d3516577c1f7b))
- gitignore .pio_cov_wsl (WSL coverage build dir) ([`ac1e36c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac1e36c6b626e9e8953a278b5ef15a7c2eaf37b3))
- update test report [skip ci] ([`17a947d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17a947db9df9f68630e6d04120c1e061a41db992))
- update CHANGELOG.md [skip ci] ([`68e5bf4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/68e5bf4f28c02cb1c30c486f3b2b975f41ac1f01))
- update test report [skip ci] ([`1726943`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17269433266f2d657e1f2daadc60ca2e376f2fad))
- update CHANGELOG.md [skip ci] ([`9ab5341`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ab53419f306c0c89d00ae5458a0e3e8bbe97548))
- update test report [skip ci] ([`95371de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/95371de3051d27d43041b2f6a1963234c8910c7f))
- update CHANGELOG.md [skip ci] ([`419ee0e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/419ee0e0d8c78a5156ab28b15473c7e22e21782d))
- update CHANGELOG.md [skip ci] ([`85c47a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/85c47a3fca0a5cfa0599b9cd390eabecca6f8b1a))
- update test report [skip ci] ([`394156f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/394156f137ea7c61e5cf904a92dabfbc5a5d2a5a))
- update CHANGELOG.md [skip ci] ([`3e1b17e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3e1b17eb8c7574fc0403ff8d2f860607afb32dcf))
- update test report [skip ci] ([`ffe7077`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ffe707766cc62bcc2161315e8929bc579cbfebed))
- update CHANGELOG.md [skip ci] ([`d56c7b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d56c7b6447e5cd84f0cfd55d0539bfdfdd314c18))
- update test report [skip ci] ([`f849efa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f849efa739fd50cf606d977cbc7ac4ef1d111026))
- update CHANGELOG.md [skip ci] ([`e9365ea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e9365eaa6fe1607985339998910d785996e28987))
- update test report [skip ci] ([`c8c5e59`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c8c5e59426acb14b1efeba86e4a52eaf5836786b))
- update CHANGELOG.md [skip ci] ([`a78277f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a78277f0b9373a2e27a9f9db6a137fd92fe0f22e))
- update test report [skip ci] ([`e0fb2ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e0fb2bacf92777913eab93b87dad99cd9ba5e151))
- update CHANGELOG.md [skip ci] ([`b262dad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b262dad184a34740734e993f9eea5ef299ab3901))
- update CHANGELOG.md [skip ci] ([`f364341`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f36434188b16d5c4e9909402f74586643f8569a6))
- update test report [skip ci] ([`f3efe73`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3efe73ae4438b47e966b9a1badb178fe733ea83))
- update CHANGELOG.md [skip ci] ([`2dc3955`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2dc3955b7170d3fedee1bcc6c663d5fb71112c47))
- update test report [skip ci] ([`62e0573`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/62e05734164168e948fe3dd7e35270744a3ce7f1))
- update CHANGELOG.md [skip ci] ([`d2e0829`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2e0829c746ae6dac642a10bfed6a7f3761a78c1))
- update test report [skip ci] ([`1192b8b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1192b8bba0d537975e582ebc7e0da7d32d1ac366))
- update CHANGELOG.md [skip ci] ([`a1feefd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a1feefd90027019a378004c732acab249046cfd8))
- update test report [skip ci] ([`1d6281c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d6281c9945afd8d5b15e5b4dede0a8ad25ed28e))
- update CHANGELOG.md [skip ci] ([`c0838ca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c0838cac988924f486d70359723acd990e1ecc56))
- update test report [skip ci] ([`b46c6ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b46c6ee158be370d783277a1206c09f809267225))
- update CHANGELOG.md [skip ci] ([`4c6c511`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c6c5119df2da9e09a2f6c84d02829ff0434cda8))
- update test report [skip ci] ([`2687695`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/268769598f8c0a565c87c1fc7abdaab82e307f80))
- update CHANGELOG.md [skip ci] ([`0db468c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0db468c2cd7a416bf2192583a32c7d6b8aa44ea5))
- update test report [skip ci] ([`d2d2f16`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2d2f16d31edbe574becce7f9b51fbde08fe787b))
- update CHANGELOG.md [skip ci] ([`3d07257`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3d07257cc4041e4ba705240e85cfc3ff38f64637))
- update test report [skip ci] ([`9fa5dd4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9fa5dd4c1ea2df9decfd8eee0e327ba1bc433cb8))
- update CHANGELOG.md [skip ci] ([`3b3f820`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b3f820c74974bc99f1b3473f92cda45989cea3b))
- update test report [skip ci] ([`3975554`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3975554645916bf1e4eafbc78b483f2488a7e341))
- update CHANGELOG.md [skip ci] ([`7795c85`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7795c85d66f3669e8ca8494cc9f97496809b8ce4))
- update test report [skip ci] ([`f6f84f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f6f84f5bce28ca516669e011978a19744b1d1cc4))
- update CHANGELOG.md [skip ci] ([`8a3ed56`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a3ed5627a4c520e17b233ae99b423eaa49af2e0))
- update test report [skip ci] ([`ae8a052`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae8a052c98741d64091df967abdfb7696668b5aa))
- update CHANGELOG.md [skip ci] ([`6a93c81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a93c81ee9818b6687b199dc32cf0f4949d19517))
- rebase-and-retry the footprint push (concurrency) ([`5f95e09`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5f95e095aa6d70816c7484cd775fa8e6aa074ea4))
- fix ESP32 Build example flag discovery + coverage exclusion path ([`e7c026e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e7c026e6e872b46f4cc5dac17d61641e6e1a735a))
- update test report [skip ci] ([`dcb7a11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dcb7a11b585cd51e3b84380e4b6027b901008890))
- update CHANGELOG.md [skip ci] ([`9a60ab6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9a60ab689a6282b08c16ba4d78ff88007254e1fb))
- bump actions/upload-artifact from 4 to 7 ([`73f2c3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/73f2c3f2e1771bffdb4445458e9e6b3bf9ec6176))
- bump SonarSource/sonarqube-scan-action from 6 to 8 ([`4fc3a82`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4fc3a8264845c4245227d89a04966beec8e57d32))
- bump actions/checkout from 4 to 7 ([`20b1ca3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/20b1ca399dc9306f7cdb11a33e0b7bf41a7c6cdd))
- bump actions/download-artifact from 4 to 8 ([`f6f50cd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f6f50cdfd56b657afd94cef6021a6689a9d32d07))
- bump actions/cache from 4 to 6 ([`06b4fef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06b4fefcd6dd452c65d11fbe38a9f97104a19f91))
- update CHANGELOG.md [skip ci] ([`c5198fc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c5198fce57568ffe73fc7837a00d487556f54e5c))
- fix coverage filter (0% regression) and pin gcovr ([`e80f2a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e80f2a265206afe90801b7d01ce459a5ea482270))
- add include_footprint.py dependency-footprint auditor ([`3ee8e81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3ee8e81e96b7ffc17b265358656b70f8770a3c2b))
- update test report [skip ci] ([`3fcea82`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3fcea8256f7f2daa3fd6d805f9431b0973708377))
- update CHANGELOG.md [skip ci] ([`eb02417`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb02417ebdf0c9f51881c335e236ad09d10532f8))
- update test report [skip ci] ([`a295a7d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a295a7d63db33c5f904fd84dbf9898653ec6bd84))
- update CHANGELOG.md [skip ci] ([`8845367`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8845367dd25b5f36dd08fa6d53756c81b6f37522))
- update test report [skip ci] ([`03faa4b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/03faa4b8a389f196d936b307e37e77251a81c83a))
- update CHANGELOG.md [skip ci] ([`e922f6f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e922f6f0d9c7c297b0df45ac64098947b228e704))
- update test report [skip ci] ([`d3d3a16`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3d3a162ca42f5d432d5d4a819ddbb44f5f35f77))
- update CHANGELOG.md [skip ci] ([`529a75a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/529a75a5fc7acaa4c8674284ed9296c32681a524))
- compile every example + harvest flash/RAM footprints ([`eb0da64`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb0da64178866ed4ad2bdc46cd779174e2a333b0))
- update test report [skip ci] ([`14b32c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14b32c5012f8c44bc6d0564ef7edfe6af38c77a0))
- update CHANGELOG.md [skip ci] ([`5c4421b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c4421b1d42c0f231b503245ced45d97a59706ad))
- analyze all of src/ recursively; exclude platform I/O from coverage ([`ba23f23`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba23f23fb919a5a0120e553061eb0fe207d34323))
- update test report [skip ci] ([`643477d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/643477ddeeacd6668a80fc4e67b53f39ddc17708))
- update CHANGELOG.md [skip ci] ([`24197c3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24197c3480227913c2e9094caff22b619b2ad490))
- compile the library for the ESP32 target (examples matrix) ([`f389b52`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f389b52f30b765603d613d48f6e96dabefee0833))
- update test report [skip ci] ([`be4f8ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be4f8ba4327bec82b44300b1977aa0b502c98198))
- update CHANGELOG.md [skip ci] ([`dd0594d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dd0594d8440f0af70b6fd97a4b4a605cfea5c3e6))
- auto-regenerate feature tables on push to main ([`e80de41`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e80de41bf2c0ad777b1cd8ecbed9c7898d6d74c4))
- update CHANGELOG.md [skip ci] ([`ffc06d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ffc06d703f686545e944fdd610bb446e6a5d0e2d))

### Changes

- Bump version: 4.64.1 → 4.65.0 ([`9f8e26c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f8e26cec76a74dfa336aa34d7c2efde006c7fb7))
- Merge pull request #7 from dstroy0/dependabot/github_actions/actions/upload-artifact-7 ([`f5c2b54`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f5c2b542b8769416b64a65e20a8b91bfcf4261a2))
- Merge pull request #8 from dstroy0/dependabot/github_actions/SonarSource/sonarqube-scan-action-8 ([`b581115`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b5811151ed8412406ebb244c2f8498336db6e9fc))
- Merge pull request #9 from dstroy0/dependabot/github_actions/actions/checkout-7 ([`f148710`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f148710a9e595d867ffbeb09a9f4fc7281ccb1ec))
- Merge pull request #10 from dstroy0/dependabot/github_actions/actions/download-artifact-8 ([`fd70ee8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd70ee8da48f358b5776a380044fd68af4662d6c))
- Merge pull request #11 from dstroy0/dependabot/github_actions/actions/cache-6 ([`16ab10a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/16ab10a55c57a00221100ea563a1b429f3f9f008))
- Revert "refactor(shim.h): move incl from src/ to src/shared_primitives/shim.h" ([`1a804b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a804b199c29f61070b9c1edd9a325bd5804df5d))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`0b15b56`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b15b568bbed839a1b1cf6ea51ac94355a3070ad))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`118d54f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/118d54f1dc59e8662458b121af839fb4ba6067cc))

### Documentation

- close #249 (shared scratch-buffer pool) - deflate window + oidc/ssh tenants migrated ([`5058a97`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5058a9772179b9d403782ef3dbb3f1d4a49f0091))
- update ESP32 build footprints [skip ci] ([`61f2bf7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/61f2bf701b8f4f9173c071d719186be009f9b390))

### Features

- DMA peripheral ingest/egress with an ingress/egress simulator ([`b5ba77b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b5ba77b3b8d25057aa156e65feb795617fa79543))
- concurrent-TLS enablement - PSRAM offload, MFL record cap, build guard ([`9e259a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9e259a76deda38884d8cdba4be00b5474986f7b6))
- enforce worker-stack RSA floor (DETWS_WORKER_STACK_RSA_MIN) when OIDC/SSH enabled ([`e3326ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e3326eef023e137d139090afde435ec654767d70))

### Refactor

- drop the shared_primitives/shim.h umbrella header ([`120b3e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/120b3e9e2c958b8791efd9e1fb762b25c022548b))
- move incl from src/ to src/shared_primitives/shim.h ([`a020147`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a020147080e93fc7ceb9e06b8fa0fd4c64fb7300))

### Testing

- cover accessor null guards, cookie parse edges, forwarded-IP whitespace/invalid ([`6a8be67`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a8be67f278b0b41ae3685f36a81bcdc853acc4c))
- cover build/parse guards and NPDU truncation rejections ([`1ba9714`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ba9714d22234004c7e3fd51dc6fabb2e22ea6f9))
- cover null guards, Long/Float metric kinds, payload fail-closed paths (100% line) ([`1f0229e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f0229ec5c0414eab29ac2a56026ccb77d49fd58))
- cover writer overflow/null guards, reader truncation paths, float-bits helper ([`3bdbcca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3bdbccada1693d4fdf593c5a1309d6f8be4cb743))
- cover init/boots accessors, encrypted discovery, tiny-cap report, priv-not-configured, notify paths ([`2273468`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2273468cddf57100c69d4be19d16156258223dbf))
- cover message truncation fail-closed sweep and priv-without-auth rejection ([`9a44424`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9a444247e45c905ca08a10ac66206d637ae6e404))
- cover slot guards, banner/build caps, KEXINIT field/trunc failures, KEXDH errors + reply overflow ([`519b2e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/519b2e0cf84d3926f1458e58c5d6c3eef1ff4d37))
- cover service/request parse truncations, RSA blob errors, build guards, DoS prefix guard (100% line) ([`cd8bbc3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cd8bbc3698cc5d2e57e1038d309974bd57ac33d5))
- cover slot/cap guards, unencrypted recv errors, seq overflow, encrypted round-trip + MAC fail ([`58ba2a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/58ba2a3c7caf863d0ad7f87c2cfd99e4f164cde7))
- cover handshake/frame codec guards, 64-bit length, host transport stubs (100% line) ([`8b48f39`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b48f397d81a03fe4ad5f839f5c19a8f5e09da48))
- cover token_kid/jwks_find/verify guards, malformed JWKS, RSA-JWK loader edges ([`ffcdf7e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ffcdf7e88169a485522c21a405763d53aef0fb16))
- cover builder guards/overflow, parser rejections, host transport stubs ([`eb019ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb019ed5aeb3071456356852586ad9a351fcf5e4))
- cover widget types, null-table guards, JSON overflow, control-parse edges ([`51af117`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51af117443ada41ce06b9ec3f17a3cd63d8cfb06))
- cover parse limits, malformed tokens, path overflow, arg-accessor edges (100% line) ([`fa5c415`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa5c4153cd24420d09744b6b7503b44608819662))
- cover all read variant types, fault rejection, malformed responses ([`6102027`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/61020272a574373a88df47da2d911d33091e02f9))
- cover parse rejections, build null/overflow guards, setters, optional read fields ([`e8ed71a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e8ed71ad7bfa29cb6a22bb14a4e9fadfe24048ad))
- cover Variant/DataValue/NodeId codec branches and reader underruns ([`09e6c50`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/09e6c50200f591c300ce8fd193de523d2221b73b))
- cover malformed options, extended delta/length, block edges, buffer limits ([`fb70d64`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb70d6472b303d57992cca9e3c95041a950cf55a))
- cover registration guards, IpAddress/value decoding, GetBulk edges, malformed PDUs ([`95b18db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/95b18db08b2b067d3b6d82ece58504d38c111ea0))
- cover URL/build/response edge rejections + host stubs ([`330d531`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/330d5317a3854f9d07db795b01d71a5a423ad41b))
- cover cbor_peek for all types, 8-byte uint, double float, map mismatch ([`beba90b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/beba90b576529cd1db4a2e34493ba4967d516fd2))
- cover malformed-token rejection, bearer spaces, claim edge paths ([`9b70a70`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b70a7071a64f310ec839b8f29658ee59bb4ff1b))
- cover base-time/none in JSON, CBOR string/bool/time, null args ([`76c33c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76c33c1013d7c5f3720c170b00ba8f774435104c))
- cover CR escape, build guards/overflow, parse edges, null lookups ([`33709d3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33709d3b5d00a8a66f055ee248dad50bf828f155))
- cover ping/pong builders, null args, put_ch overflow, parse edges ([`0a63126`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a6312611e3c4337b79d00291e15dc8e13134e32))
- cover all varbind value types, invalid type, null args, stubs ([`e44b4a9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e44b4a9c4e7ebc7ba4fd8d67b59baeb6c49c2d68))
- cover init/log via UDP capture, PRI clamps, field truncation ([`22ee59f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/22ee59f58747ffba4fbe7d812786169e6434ae55))
- add det_numparse + det_utf8 suite (new native env) ([`066bc2f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/066bc2f0b95145d3f478895e5d1d036d0b86e6de))
- cover JSON escapes, category names, and fail-closed buffers ([`bd58bbc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd58bbc7540e03ddc17d003f71ce73f2ce79e870))
- cover unknown slot, CR/control, IAC-escape, subneg, capacity, output ([`fcea5bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fcea5bcf1fd5998afe3f3d615ae4cfda22f60925))
- cover builder/parser guards, all DIF codings, record-walk edges ([`34c350d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/34c350d4421c2654fc52f41035393b49dbac29a0))
- cover accessors, per-FC exceptions, small buffers, RTU edges ([`6eb3761`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6eb3761ee783da9c3224466bdfe29028c8844197))
- cover escapes, null guards, hex-case, false, malformed objects ([`8d40531`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8d40531ed2fdc20ffba629f59f789bc70c53b47f))
- cover all builders, null/overflow guards, typed-parse rejects ([`8e437f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e437f6375c0b969f31d17ed8aa75d24fceb87d2))
- cover error paths, full COB-ID classification, SDO variants ([`14f919c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14f919cab8fdc6e1497e7881da3bf0828066c3df))
- cover graphql edge paths + SSH ext-info branches (coverage) ([`64e419b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64e419b1e80f3c8323b24b2863c1ee9c02fbc72c))

</details>

## [4.64.1] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.64.1 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`275518c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/275518c9688c2440fa6517eb62cefab3b3e5e9fd))
- update CHANGELOG.md [skip ci] ([`3d57de5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3d57de5af5395ad553416d73b18a140ed046029f))

### Changes

- Bump version: 4.64.0 → 4.64.1 ([`64d2873`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64d2873a45e81e76ff9983d3dc80ddc0914a31cb))

### Documentation

- regenerate feature tables; roadmap (build configurator, SSH ECC) ([`c797287`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c797287908bca04c38132c00912e5be670e1025e))

</details>

## [4.64.0] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.64.0 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`d1d79f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1d79f62766b7977f64b1fec0669b870b0b1accf))
- update CHANGELOG.md [skip ci] ([`5a404ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a404ab10672047757ba2c3cdab0812db293c219))

### Changes

- Bump version: 4.63.0 → 4.64.0 ([`abf635a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/abf635a0af50e08b6ac4911dbe91f953e874c371))

### Features

- TCP port forwarding (ssh -L) + OpenSSH interoperability ([`955191a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/955191a7fafc2f921c1a43a382c65027315ca4eb))

</details>

## [4.63.0] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.63.0 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`f166889`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f166889f3690147cd9c42279ff3849d1fa84cf61))
- update CHANGELOG.md [skip ci] ([`4e160bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e160bb6cffde658251856e0ac5f64a90a1323c6))

### Changes

- Bump version: 4.62.0 → 4.63.0 ([`6dd6bf3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6dd6bf314a903340f18b53096c97737ccd752ca1))

### Features

- parse and route direct-tcpip forwards through a normalized seam ([`c43fdac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c43fdac978b616a5bc3a92c184577b2bdf82222e))

### Testing

- add a known, public, test-only RSA-2048 host key fixture ([`4dbe2d8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4dbe2d81226a4f8fa608a9b67d1f645dfdfcd0a2))

</details>

## [4.62.0] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.62.0 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`8afbd55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8afbd555b4b365c1fab1a7b8e050be99d4c5ba60))
- update CHANGELOG.md [skip ci] ([`49e9a23`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/49e9a236f7bea932cb0484c75e7650b2ff775d49))

### Changes

- Bump version: 4.61.0 → 4.62.0 ([`e6405f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e6405f7ef6b3e0335a5a169ad9e8e92dc9f82dfe))

### Features

- multiplex SSH channels (per-connection channel table) ([`513dc68`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/513dc68939a73e83b4aab633b319e340265527de))

</details>

## [4.61.0] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.61.0 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`aee0a98`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aee0a98a4e74096ed79d355de2dc26b34c146d9c))
- update CHANGELOG.md [skip ci] ([`1c0cce3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c0cce38dbf59a23bc7bd756f44ccbf3019b1895))

### Changes

- Bump version: 4.60.0 → 4.61.0 ([`3824a80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3824a80c0120da12117186bb6ca4aed7014d29fa))

### Features

- v5 clock-awareness - microsecond base + latency budgeting ([`546c984`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/546c984c3b518cc3e7c47864857e6cf46343a7e2))

</details>

## [4.60.0] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.60.0 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`2753752`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/275375207e89619ce783c1930ce4f0b0a7af1281))
- update CHANGELOG.md [skip ci] ([`c9dfa20`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c9dfa20ee5737e217914dd84d39188b82da980ca))

### Changes

- Bump version: 4.59.18 → 4.60.0 ([`68df75c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/68df75c330650292ff3494e0d5d9230525648539))

### Features

- v5 preempting work queue (ISR/task -> high-priority task) ([`9411f99`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9411f99cc1c0b97ce90f8f09126a323e37a736c3))

</details>

## [4.59.18] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.18 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`8adf76d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8adf76d1b1ce501a9a8657a186185b32cb204328))
- update CHANGELOG.md [skip ci] ([`0e0b6cc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e0b6ccde2ec049fb10336fcc9408314fe18fabd))

### Changes

- Bump version: 4.59.17 → 4.59.18 ([`8f83fa7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f83fa7c3acc57c4b3a4dee6bacca1b0bb0bb349))

### Documentation

- record actual SonarCloud coverage (~68%) and why it differs from gcovr ([`de2cca1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/de2cca1145ed567a2623e1a44b1899678919af9b))

</details>

## [4.59.17] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.17 - 2026-06-30</b></summary>

### CI / Build

- coverage via per-env reports merged (fixes the scan-breaking report) ([`6711c66`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6711c66a2e03090f1171f809baccf13d29fa3757))
- update test report [skip ci] ([`393ada0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/393ada043cc940d1a13e269924a8d4ea72449096))
- update CHANGELOG.md [skip ci] ([`312ef6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/312ef6eadfa42315c19fc6592c3977917d1a7521))

### Changes

- Bump version: 4.59.16 → 4.59.17 ([`0da14e6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0da14e6dafb9b3f57846381509a9c1500ef72556))

</details>

## [4.59.16] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.16 - 2026-06-30</b></summary>

### CI / Build

- temporarily disable coverage import (report tripped the parser) ([`33b8bdf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33b8bdf37dcdd51b53186d0ed1df4dd09d29385c))
- update test report [skip ci] ([`29b6abe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/29b6abe8241b909f565333a9b3a166ebcf23406f))
- update CHANGELOG.md [skip ci] ([`96fd09f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96fd09f42d11689d7bd40cc29b429f545b6b56d3))

### Changes

- Bump version: 4.59.15 → 4.59.16 ([`cb246a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cb246a2d7658e8eec67bce5e005fbf15b0bdfdae))

</details>

## [4.59.15] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.15 - 2026-06-30</b></summary>

### CI / Build

- import test coverage into SonarCloud (gcov + gcovr) ([`b097637`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b097637eaa14c1b0fa1fa6de4e2b7a2bfc34e931))
- update test report [skip ci] ([`f1f386a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f1f386a2d6e75ee759bbc6f9f5cb68d712759cc3))
- update CHANGELOG.md [skip ci] ([`f8e98c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8e98c0cb9b94acc6317763b719ad4ce1c1feacf))

### Changes

- Bump version: 4.59.14 → 4.59.15 ([`ecd9d31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ecd9d31494dbf51d0152dea52e017faa45e837f3))

</details>

## [4.59.14] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.14 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`97e9e28`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/97e9e2877a9a08066cfef4547fe1ae96158ec160))
- update CHANGELOG.md [skip ci] ([`37e4212`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/37e4212ad12020da6899ad04623c1432bf3ab7c1))

### Changes

- Bump version: 4.59.13 → 4.59.14 ([`546d99d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/546d99d08c364a3619807ef220cd496f2c1f1a5d))

### Documentation

- add SonarCloud quality-gate badge, group badges by category ([`d5d1789`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d5d17893a01b1fcb11450c0d7f8c9634fd7d4c7c))

</details>

## [4.59.13] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.13 - 2026-06-30</b></summary>

### Bug Fixes

- initialize byte before the ring read (cpp:S836) ([`71f5b44`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71f5b44fb78b34c38e153940c58f432064ccb838))

### CI / Build

- update test report [skip ci] ([`a7da098`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a7da0985d1e1fc6cb37a8c978ee950920403e141))
- update CHANGELOG.md [skip ci] ([`2286cfa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2286cfafed31d5cef498f07f7158316afeee7ad5))

### Changes

- Bump version: 4.59.12 → 4.59.13 ([`1290982`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12909824b67d2f1b392535e9ccec2d832e697602))

</details>

## [4.59.12] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.12 - 2026-06-30</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`8bdad91`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8bdad9117c7c04fc74e594a03250f84ce3f02f86))

### Changes

- Bump version: 4.59.11 → 4.59.12 ([`43dae17`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/43dae170bc3ead75f5160fefcf195a4ead2409e0))

### Documentation

- note the ~7KB RSA modexp stack floor at DETWS_WORKER_TASK_STACK ([`f48b2b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f48b2b4f85ec015a3fd5abbc6e2784493bef4676))

</details>

## [4.59.11] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.11 - 2026-06-30</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`165e542`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/165e54263c008af6fdfae604ffd15aa48fd85b0e))

### Changes

- Bump version: 4.59.10 → 4.59.11 ([`08d208b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08d208b9b65049719506745f074d13026639146d))

### Testing

- sync test_matrix.json with the native_oidc scratch/worker srcs ([`d05811b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d05811b0dbf8344ea4ad1d99d1c145bb834fd2f3))

</details>

## [4.59.10] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.10 - 2026-06-30</b></summary>

### CI / Build

- update test report [skip ci] ([`2a3fe96`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2a3fe96e2f3440f21a341213662e95fc447db5a7))
- update CHANGELOG.md [skip ci] ([`23b39d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23b39d78f7a3c9953ad20c398b190f8c6d29e140))

### Changes

- Bump version: 4.59.9 → 4.59.10 ([`751acd9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/751acd95f1e3046d155472a33675440c6ac601b0))

### Refactor

- borrow the verifier decode buffers from the scratch arena ([`25bf1f3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25bf1f35e5e36ec599524c6df89cf058e007030f))

</details>

## [4.59.9] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.9 - 2026-06-30</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`01e063c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01e063c8a3cb4006be8a84bace0c9ff8222ea0e6))

### Changes

- Bump version: 4.59.8 → 4.59.9 ([`28de880`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/28de8809cdc3eafc6c55ad77e1167224494bab66))

### Documentation

- record the code-smell quality-profile guidance ([`a1b2030`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a1b203059b61d7542608f58457f7533280c53ec3))

</details>

## [4.59.8] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.8 - 2026-06-30</b></summary>

### Bug Fixes

- drop the side effect from the && in the match-chain loop ([`d671441`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d6714410134278304d95eac55b2848ce982f7512))

### CI / Build

- update test report [skip ci] ([`a876120`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a876120eb8c97dce6a3e1b9cdd45fd2102b29a55))
- update CHANGELOG.md [skip ci] ([`7a0f5b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a0f5b342eac51aebd542fe0b59ea19768e748f2))

### Changes

- Bump version: 4.59.7 → 4.59.8 ([`2c5c2e2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c5c2e233ad4c13a51754bd640ac159e50144a35))

</details>

## [4.59.7] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.7 - 2026-06-30</b></summary>

### Bug Fixes

- resolve SonarCloud bugs/vulnerabilities from the first scan ([`91eb912`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91eb912722371d661ea1ced32939cb9f853d2c26))

### CI / Build

- update test report [skip ci] ([`451149f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/451149f25329f73724d848c7c1d01726edd8bf93))
- update CHANGELOG.md [skip ci] ([`00d81d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00d81d440db7936f1dc8aa503d89651e74d07cd1))

### Changes

- Bump version: 4.59.6 → 4.59.7 ([`8b5b412`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b5b412b0256f963b5cb7c970634478522b26a11))

</details>

## [4.59.6] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.6 - 2026-06-30</b></summary>

### CI / Build

- bump sonarqube-scan-action to v6, add scan concurrency guard ([`9210215`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9210215e44d1638bf75e2ab98cc63b91da5ae9b2))
- update CHANGELOG.md [skip ci] ([`80c8516`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/80c85166ed0b70cee0a26e88fff438f4c92414f6))

### Changes

- Bump version: 4.59.5 → 4.59.6 ([`dde78fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dde78fbd5c7c5b1481649156794ae66df5ff4836))

</details>

## [4.59.5] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.5 - 2026-06-30</b></summary>

### CI / Build

- SonarCloud C/C++ analysis with a merged compile database ([`040f9a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/040f9a70533fd5143e4f190a966689a5b96274da))
- update test report [skip ci] ([`15f51cc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/15f51cc3ec448bc4f98370b1bcbf7d508fa9021f))
- update CHANGELOG.md [skip ci] ([`508c7bd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/508c7bd3b7e4aea66564d280741576195822515e))

### Changes

- Bump version: 4.59.4 → 4.59.5 ([`9329711`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9329711621c6f17bce09dee00262ad8ebfc91bb7))

</details>

## [4.59.4] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.4 - 2026-06-30</b></summary>

### Bug Fixes

- resolve the open CodeQL warnings ([`401a4e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/401a4e527f966eb85547882b0a9d4ef2eff70c6d))

### CI / Build

- update test report [skip ci] ([`c2aeaa5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c2aeaa5e3473e6da8c7a15c378d944d5e7d5b8a3))
- update CHANGELOG.md [skip ci] ([`c5d3cfc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c5d3cfcf52b159a3b6edb6e4bf3343ce05d35b23))

### Changes

- Bump version: 4.59.3 → 4.59.4 ([`ae06e93`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae06e93434f826856b29ede47eb9d4e619291305))

### Refactor

- slot-owned connection teardown, no raw pcb in L7 ([`0e7f04e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e7f04ee4f3f2bf8335dde96154fc2914dd5f8a7))

</details>

## [4.59.3] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.3 - 2026-06-30</b></summary>

### Bug Fixes

- stop run_tests.sh spewing "tail: write error: Broken pipe" ([`76dc6a8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76dc6a8aacc8c8d5a77e8227a6bb51d11097789e))

### CI / Build

- update test report [skip ci] ([`26fcedb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/26fcedb052c62ff9bab9dddb2794ac7f2827d9d6))
- update CHANGELOG.md [skip ci] ([`28fe8a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/28fe8a10d7bdf446005c00e00c2e6412c9ce3873))

### Changes

- Bump version: 4.59.2 → 4.59.3 ([`89faec1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89faec1eb0d3d32e335f9568d0dd9b0eb519555c))

</details>

## [4.59.2] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.2 - 2026-06-30</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`76d7bbc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76d7bbc890dd1ec4f044d45de141101bdf91509b))

### Changes

- Bump version: 4.59.1 → 4.59.2 ([`a74b779`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a74b7797b5b9cfabb4b46a1a59455a42a6bc8084))
- clang-format shim.h include block ([`415cecd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/415cecde8241e49116b35d9c9fd82f176979602c))

</details>

## [4.59.1] - 2026-06-30

<details>
<summary><b>Show Changelog for version 4.59.1 - 2026-06-30</b></summary>

### Bug Fixes

- NUL-terminate the built ASCII frame ([`230e32b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/230e32b92924c93d770300932b0ed1d35eaf47a4))

### CI / Build

- update test report [skip ci] ([`6924bb3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6924bb320f0de76fabd97d3dde40d79a4be4dae4))
- update CHANGELOG.md [skip ci] ([`1288a33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1288a3376df2ebf70ce96015552db0c6b814ad1a))

### Changes

- Bump version: 4.59.0 → 4.59.1 ([`f652a57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f652a5765e090c119af10414b74a325b82c4561b))

### Documentation

- add interface forwarding (v5) + post-v5 southbound bridges ([`7ed4bcc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ed4bcc32803cf8d38d113778766b32f7b950e74))

### Refactor

- centralize std headers behind shim.h ([`bd1288a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd1288a6966145fd10b4c78b5447c45cc564c165))

</details>

## [4.59.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.59.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`8b4903b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b4903bdd8c7326b11aa4e645be6a24b49c82129))
- update CHANGELOG.md [skip ci] ([`bc1a918`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bc1a9181cb5575a6aa9e103c4904922593944e2b))

### Changes

- Bump version: 4.58.0 → 4.59.0 ([`67515d3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/67515d33f83e09e6d1eea9da1cff5f355ee060ae))

### Features

- IO-Link (SDCI) data-link message codec ([`c4383b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4383b15d5ac7077c87d4801104436d43279c38d))

</details>

## [4.58.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.58.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`189316d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/189316d539bc7003b7c10463a47bcbf6353863af))
- update CHANGELOG.md [skip ci] ([`31b3fda`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/31b3fda0829dbc991495c94705a286210bbcfc0d))

### Changes

- Bump version: 4.57.0 → 4.58.0 ([`692694d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/692694d8a27a125d6a189eba8a9b00947f3556c4))

### Features

- NMEA 0183 marine / GPS sentence codec ([`fd8df3d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd8df3dcfe8213d2a82acb53f099cbe9922d04fe))

</details>

## [4.57.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.57.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`69d00c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/69d00c000ffbb21f15e4cb3b16b8e57136c2db35))
- update CHANGELOG.md [skip ci] ([`b435fbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b435fbef12c00381c4873a24afea5c03c7549baa))

### Changes

- Bump version: 4.56.0 → 4.57.0 ([`853a093`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/853a09343e62beb69d646758e3d36bd77b4efbd3))

### Features

- DMX512 framing + RDM (ANSI E1.20) lighting codec ([`4b477c4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b477c43bb77fa2227f728e34014b08db2154ca1))

</details>

## [4.56.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.56.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`18093c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/18093c8b49471577bf849bc4e54c08a449d9cf2d))
- update CHANGELOG.md [skip ci] ([`282cef3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/282cef3e48c8592cacd5bbdfa972fcfa1b1cbbc3))

### Changes

- Bump version: 4.55.0 → 4.56.0 ([`cdf94f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cdf94f5b14add28ee082ee6e8fcd18f3d9d43a35))

### Features

- SDI-12 sensor-bus command / response codec ([`4ef0a90`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ef0a90bb76f65b5522b681ae467f82c78f78cf3))

</details>

## [4.55.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.55.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`6699c0b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6699c0b72a6098c893faa956b8968b6acc68f613))
- update CHANGELOG.md [skip ci] ([`fb482f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb482f6691d5b536e03fa6ffc19a2b8ee0b2d7d6))

### Changes

- Bump version: 4.54.0 → 4.55.0 ([`38ba000`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/38ba0006e9f1d00bf645d62bf4a1455af1c1e146))

### Features

- IEC 60870-5-101/-104 telecontrol (SCADA) codec ([`17b27f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17b27f6bdecfa650ccad285785fe792098943f49))

</details>

## [4.54.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.54.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`3dcd433`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3dcd433fbef47d60dcfe7eb7c82e1b9eb6ad0097))
- update CHANGELOG.md [skip ci] ([`ec35755`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ec3575501fc28681e257bb9c80de04cd313f8d27))

### Changes

- Bump version: 4.53.0 → 4.54.0 ([`ec8c2c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ec8c2c6db464fc6ec4a26a701af11a603bc18432))

### Features

- wired M-Bus (EN 13757) frame + data-record codec ([`3874e97`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3874e97c23950c83df403cdad2874a58663ad9ca))

</details>

## [4.53.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.53.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`ef9a27e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef9a27ecf09a50f3e127fd15eac99ab4e8710ac1))
- update CHANGELOG.md [skip ci] ([`e289a5a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e289a5a0c05704636566b81806c48bf1eb7b5a45))

### Changes

- Bump version: 4.52.0 → 4.53.0 ([`e904459`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e904459853284ae4e53db81f08212e34123c6232))

### Features

- NMEA 2000 codec (Fast Packet over J1939) ([`192101b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/192101bba345fcf90ba080eab2a54634af39e78a))

</details>

## [4.52.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.52.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`47da9f0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/47da9f0250ba951b56a16dca6e2a2dc5dc04810e))
- update CHANGELOG.md [skip ci] ([`14071c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14071c8c35333d591b7af9c0f0215a29f65be44e))

### Changes

- Bump version: 4.51.0 → 4.52.0 ([`92216c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92216c963a4b5468619c91e777d5b816028c4263))

### Features

- DeviceNet link-adaptation codec (CIP over CAN) ([`e5feb37`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e5feb3710525477c94f20215491ea745722bc51e))

</details>

## [4.51.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.51.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`b8fcb6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b8fcb6e80ce2e602dcc1beb1790058d44e9f1680))
- update CHANGELOG.md [skip ci] ([`288b114`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/288b114590b489c4f7932f12360ead4a43559d03))

### Changes

- Bump version: 4.50.0 → 4.51.0 ([`c983a88`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c983a882236964b4803d6591aebee56b3f55afc8))

### Features

- SAE J1939 codec over 29-bit CAN ([`6535135`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6535135601ca3f71eb827437cfc7c57095819e3d))

</details>

## [4.50.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.50.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`6073f0a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6073f0ad1f2ebf31f19d649140e7b7aaf58650c6))
- update CHANGELOG.md [skip ci] ([`147834d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/147834d7c1b13a8f7042fe8e10ff5489eac3a8ab))

### Changes

- Bump version: 4.49.9 → 4.50.0 ([`a61819d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a61819de59f42aafe874867c62d024277d05027a))

### Features

- CANopen (CiA 301) message codec over CAN ([`8330d6f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8330d6f3df8f2c8872d34739f283900f89f64fc8))

</details>

## [4.49.9] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.9 - 2026-06-29</b></summary>

### CI / Build

- add memmove and varints to the dictionary ([`1d4e06a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d4e06a679a723794502e05c5dfaf131d79aca18))
- update test report [skip ci] ([`9fa0dc2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9fa0dc2577ba7e304ad48f9ce742b7398f8ce725))
- update CHANGELOG.md [skip ci] ([`d4bb8de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d4bb8ded1655a9cec44537aa8e78e9bab3ac29c7))

### Changes

- Bump version: 4.49.8 → 4.49.9 ([`bfd1c64`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bfd1c64aa21d4c8ad3f78916bc6ce99a33c3d407))

</details>

## [4.49.8] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.8 - 2026-06-29</b></summary>

### Bug Fixes

- wrap-safe length bounds in SNMP BER + HTTP chunked decode ([`626372c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/626372c436c89fcf38346f2e33191fea941cbb74))

### CI / Build

- update test report [skip ci] ([`9486b7e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9486b7e5427bac7cebf3d0898343479230460fbc))
- update CHANGELOG.md [skip ci] ([`968a402`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/968a4024b1bc68f3d9abb1a854175675cb1404fe))

### Changes

- Bump version: 4.49.7 → 4.49.8 ([`b30a297`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b30a2977f7bc1250b9cb9d1176da4cb4bfab61e0))

### Refactor

- use reentrant gmtime_r everywhere (worker-safe) ([`804a8a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/804a8a1bf6e607d4163b13daec963fc00c262270))

</details>

## [4.49.7] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.7 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`2fe507d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2fe507d73e0f7764465426e72cc6b7fdbda3b331))
- update CHANGELOG.md [skip ci] ([`675fa61`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/675fa61005631404485a3e2661599f520ac04f4b))

### Changes

- Bump version: 4.49.6 → 4.49.7 ([`f34a77e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f34a77efd50dbc568bd884ce1e8ca78e70e49dbc))

### Documentation

- log the OIDC stack frame and L7 tcp_pcb teardown as debt ([`347b307`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/347b307e1380d50cf8a15a817d32a669d9906850))

### Refactor

- gate csrf/auth_lockout behind their feature flags ([`cea8a25`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cea8a25f07416206cbe3b65cdfe25b25b873d32d))
- name DNP3/MELSEC/COTP frame-geometry constants ([`b38cf9d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b38cf9df885dcaf907469900717f2775bf4783da))
- drop the now-dead lwIP include (close an L7 leak) ([`7de4bf8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7de4bf897c96f2b8a39bdc00c110649635490540))

</details>

## [4.49.6] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.6 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`cd60e27`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cd60e2707e6bced502e2fb9bfc88bd8ec390ecfe))
- update CHANGELOG.md [skip ci] ([`d379cb7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d379cb70464d275238f0fdd76dd2eda04a4a5046))

### Changes

- Bump version: 4.49.5 → 4.49.6 ([`6271fce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6271fcec3603193e2f28289588152434239d3e13))

### Documentation

- generate README feature/codec tables from FEATURES.md ([`262fba3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/262fba3ce47bc3920b32896fc809deca57b7af1d))

</details>

## [4.49.5] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.5 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`71e4e15`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71e4e152e9c143adef78d202bf4ce79c24e5b04b))
- update CHANGELOG.md [skip ci] ([`a4df0e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4df0e4ca82a21480187d937443d91c01b5d18c3))

### Changes

- Bump version: 4.49.4 → 4.49.5 ([`79ec4c7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/79ec4c75d779bded88f81956a4cffabb080d2c4c))

### Documentation

- add hardware hookup & settings guide for external-hardware codecs ([`905519b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/905519b280c19daee0212aac00193570551dcdfb))

</details>

## [4.49.4] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.4 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`bd349b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd349b6e86a48877c25c44d064499df4bb3ffacb))
- update CHANGELOG.md [skip ci] ([`d1827f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1827f5f5fd013b3aae97d5de11025c06f0ccecd))

### Changes

- Bump version: 4.49.3 → 4.49.4 ([`5e45c8f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e45c8fdd7347cb1b98bc526b076baa99b0eb96f))

### Testing

- close codec edge-case gaps (varint width, TLV lengths, WAMP URI) ([`439d2e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/439d2e454fd5adebd1bcd41a4182bb7528f04cec))

</details>

## [4.49.3] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.3 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`5b64d35`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b64d35ccf417c76c9d92bce91c6db3f5ed53d65))
- update CHANGELOG.md [skip ci] ([`59669df`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/59669dfb8cf686b0a5dbbccae32f65d0fb700ed5))

### Changes

- Bump version: 4.49.2 → 4.49.3 ([`a67f942`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a67f9429234fea8574419b4a3e9572c99aa3b2be))

### Testing

- cover the accept-time connection gates (audit gap) ([`55b64ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/55b64ec7416c3b34c2c42d3becaeb97ac8396fff))

</details>

## [4.49.2] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.2 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`dd97c57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dd97c57ac5bbdbc5dca7160147460f051c71abe5))
- update CHANGELOG.md [skip ci] ([`a421790`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4217908d831a24e0b188024a011adfc347c32a1))

### Changes

- Bump version: 4.49.1 → 4.49.2 ([`c34ae40`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c34ae4098dc45c385f79b074fbdae6ab19746b50))

### Documentation

- close documentation-audit coverage gaps ([`4e78483`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e7848326af84f80de714c1be1b70a9188d3e535))

</details>

## [4.49.1] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.1 - 2026-06-29</b></summary>

### Bug Fixes

- harden length-field parsing against 32-bit size_t overflow ([`785e6d8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/785e6d8fbaa3e4bac5f7e624ef04355bbaa8653c))

### CI / Build

- update test report [skip ci] ([`21b16c7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/21b16c7f8a1cbc14f2d43d7ba0d60f895980d808))
- update CHANGELOG.md [skip ci] ([`89260d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89260d9ffa14430af6e940d4b0aa33a20cf56abe))

### Changes

- Bump version: 4.49.0 → 4.49.1 ([`5915141`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/591514178e017cf124ee40ed74aa4e93dd8abe88))

</details>

## [4.49.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.49.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`1d15c72`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d15c72636e28273e226541de56e977b0ba4151d))
- update CHANGELOG.md [skip ci] ([`9b614f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b614f2920193467c8d9951e83b995a48bf044bc))

### Changes

- Bump version: 4.48.0 → 4.49.0 ([`3cb8bcd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3cb8bcda064fd5024438263e02d723b918c4e81e))

### Features

- Sparkplug B payload + topic codec (DETWS_ENABLE_SPARKPLUG) ([`e5e43a4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e5e43a45686dd8522a8869ae9c0e1505a3536c2f))

</details>

## [4.48.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.48.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`af8ebf5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/af8ebf5dd3bcaf1b93b16d3ab71bf913f9868dc2))
- update CHANGELOG.md [skip ci] ([`e542f8b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e542f8b10c29a8d18696545283a1d339fad03b13))

### Changes

- Bump version: 4.47.0 → 4.48.0 ([`1a5abdb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a5abdb7164f452860c1a79d19df7dea6c8ed27c))

### Features

- HAProxy PROXY protocol v1/v2 codec (DETWS_ENABLE_PROXY_PROTOCOL) ([`b0e4674`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b0e46746983917d400fc1d8eae0c33864c9ac132))

</details>

## [4.47.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.47.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`1d87137`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d871375fdcd456ef352b2240966e0277683ef73))
- update CHANGELOG.md [skip ci] ([`0d2c69f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d2c69f5b2ccd751c956aba8f7d05f56991f8e8d))

### Changes

- Bump version: 4.46.0 → 4.47.0 ([`132ae14`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/132ae14463bd64ee27446ca64b6d11e00bbf4d69))

### Features

- NATS client protocol codec (DETWS_ENABLE_NATS) ([`3110c64`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3110c64173bb10ea6020b07fd47172b042f06264))

</details>

## [4.46.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.46.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`5986372`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5986372d7ee7ccd876d9bed217a3a170e588c743))
- update CHANGELOG.md [skip ci] ([`fee8a4e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fee8a4e4dd07fa1857b12b49e751c50edd2875c6))

### Changes

- Bump version: 4.45.0 → 4.46.0 ([`f12447b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f12447b5c1023f028327c93a131a45797172c34f))

### Features

- CIP message codec (DETWS_ENABLE_CIP) ([`925efe8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/925efe867099ad9bcdb08af4d61812063a3e23a7))

</details>

## [4.45.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.45.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`f21be0d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f21be0d212538de5b5e8f22e9d4c199f018281c7))
- update CHANGELOG.md [skip ci] ([`0ca02dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ca02dd101ba48af61d17b1034c1125691edcf49))

### Changes

- Bump version: 4.44.0 → 4.45.0 ([`573f6b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/573f6b2bbddc3b456a6ecac901b7724b0096b8cf))

### Features

- AMQP 0-9-1 frame codec (DETWS_ENABLE_AMQP) ([`be0bc98`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be0bc98ddc434663ed56d3b00a935fc5548c8b00))

</details>

## [4.44.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.44.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`4a012b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a012b40c650cd45d18ff01bd674202eb178a9a5))
- update CHANGELOG.md [skip ci] ([`d423288`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d423288345445daa55eb435ac65259a037750aa3))

### Changes

- Bump version: 4.43.0 → 4.44.0 ([`4a95ef6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a95ef6acfac1004382041c77a93a67fcd3a783a))

### Features

- EtherNet/IP encapsulation codec (DETWS_ENABLE_ENIP) ([`55a4124`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/55a4124ec1d8ba1e73a6df768af6694cf35dee6b))

</details>

## [4.43.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.43.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`1021dfb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1021dfb99231967c196336ee0e57ae23a2b4dc58))
- update CHANGELOG.md [skip ci] ([`d9874aa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d9874aa5adfeaf42d15252f2dc3c0dea05eb1147))

### Changes

- Bump version: 4.42.0 → 4.43.0 ([`4f49a1b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f49a1b9c880904c2876bdfc086cf86dae79f2fa))

### Features

- BACnet/IP BVLC + NPDU codec (DETWS_ENABLE_BACNET) ([`2e8f209`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2e8f209cf7d50ec88747b9961013a354e111d7cc))

</details>

## [4.42.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.42.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`142c45b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/142c45ba7992186b07c4034b5c330d41b643f2e2))
- update CHANGELOG.md [skip ci] ([`f30a2ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f30a2ab2104cb85dcbd345a716aaf4512a99a5e1))

### Changes

- Bump version: 4.41.0 → 4.42.0 ([`5053deb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5053deb8f08a6fc97ff739edaaa38cac4a6a48ba))

### Features

- Mitsubishi MELSEC MC binary 3E codec (DETWS_ENABLE_MELSEC) ([`ca230e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ca230e8227da548c02b25b59cbda74db4364aa5a))

</details>

## [4.41.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.41.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`b05b856`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b05b856c35a0508c31d678b6168d3d9e56da1190))
- update CHANGELOG.md [skip ci] ([`b823f83`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b823f83f3b07813119ab763f72675e2191633820))

### Changes

- Bump version: 4.40.0 → 4.41.0 ([`693fb0e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/693fb0eaebd8840f1f0b148a52775f231c60ef8a))

### Features

- Siemens S7comm PDU codec (DETWS_ENABLE_S7COMM) ([`4026fd8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4026fd8550a190046ce6a834f3fe59384aae8376))

</details>

## [4.40.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.40.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`f647b2e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f647b2e3e3f009394edca1359fb2ed15aebdc431))
- update CHANGELOG.md [skip ci] ([`d5fce4b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d5fce4bfcaa9c7b9f8f5c96e90415a499662178c))

### Changes

- Bump version: 4.39.0 → 4.40.0 ([`991886e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/991886e5986dccf8a9670c92d21754c7d2b5e4c8))

### Features

- TPKT (RFC 1006) + COTP X.224 frame codec (DETWS_ENABLE_COTP) ([`7039c9f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7039c9f01c269baec72bd44a1a96c91d88157903))

</details>

## [4.39.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.39.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`1471224`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1471224f17d3e1396dd94cf89423f6c3a7aad563))
- update CHANGELOG.md [skip ci] ([`f3e3d5c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3e3d5cc971d8837aee101f4fbf5cb91029244c3))

### Changes

- Bump version: 4.38.0 → 4.39.0 ([`e0696c7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e0696c72407ef45931e7363627c4199b59cab2c6))

### Features

- Allen-Bradley DF1 full-duplex frame codec (DETWS_ENABLE_DF1) ([`23780e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23780e5530e694e07a32204f985c6b86e4bd68c1))

</details>

## [4.38.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.38.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`3edf678`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3edf678f74f560d0c50d0f924b161dd90e72f80a))
- update CHANGELOG.md [skip ci] ([`a67e20b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a67e20bc6f1a18a0c92d14c0ecf3cdfb56578565))

### Changes

- Bump version: 4.37.0 → 4.38.0 ([`a3ce4c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3ce4c10aca8e4c2353c0e51262125424dd00697))

### Features

- SenML (RFC 8428) pack builder (DETWS_ENABLE_SENML) ([`f2d18e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2d18e8437484bb7d7bc34396f2a4457d76f6998))

</details>

## [4.37.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.37.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`36d910d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36d910d459a743e5dd2083550ddb290d6747be3f))
- update CHANGELOG.md [skip ci] ([`509b3cc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/509b3cc159fc18fcf38cbe186e798cddef3b7ed9))

### Changes

- Bump version: 4.36.0 → 4.37.0 ([`cb46a41`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cb46a414c296442c4e958d95566be0eff4a5e79f))

### Features

- Omron Host Link (C-mode) frame codec (DETWS_ENABLE_HOSTLINK) ([`6e60fc7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6e60fc70b6382adc90a19ea87fa67a877da14e2a))

</details>

## [4.36.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.36.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`d34f5ea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d34f5eadb939302a5a1bf1dd5d2be0181ffc20b0))
- update CHANGELOG.md [skip ci] ([`bae9d5c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bae9d5c81d0d9091384f9b0ce30b61006f792ffe))

### Changes

- Bump version: 4.35.0 → 4.36.0 ([`083e6ac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/083e6acca288c8500ef49179902893028def948c))

### Features

- Omron FINS frame codec (DETWS_ENABLE_FINS) ([`b1f1d1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b1f1d1c7ebbb7c1e48841919a9306e9e8f5dd7dd))

</details>

## [4.35.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.35.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`b874c3c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b874c3cb416dcb5660d1c7681773bd93158a61b5))
- update CHANGELOG.md [skip ci] ([`84ecfc4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84ecfc44a6dba8f41ce9f6094608abc6e2ae7932))

### Changes

- Bump version: 4.34.0 → 4.35.0 ([`51b7dad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51b7dada913723febc37e81905ec71ddf37a9abf))

### Features

- OMA LwM2M TLV codec (DETWS_ENABLE_LWM2M) ([`77abb7e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/77abb7ea92bea1dc68ace24fb4f86836caa345d5))

</details>

## [4.34.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.34.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`7cc32ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7cc32abf647e3e3329c4925880bc5cf20bde90cd))
- update CHANGELOG.md [skip ci] ([`773c9c4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/773c9c4817b536e2d4deb5648ccc1a4819a0af71))

### Changes

- Bump version: 4.33.0 → 4.34.0 ([`fd6300f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd6300f147d6970713c930cbed9204069742d7e1))

### Features

- gRPC-Web message framing codec (DETWS_ENABLE_GRPC_WEB) ([`f041f26`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f041f266f67c33016cbaf762c7bf498392c9890b))

</details>

## [4.33.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.33.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`f126c4e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f126c4e557ab76d47db090dbb81ab0ad5ef53a59))
- update CHANGELOG.md [skip ci] ([`04541f9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04541f978dba7bd96c756ef8bdeec0b057891c74))

### Changes

- Bump version: 4.32.0 → 4.33.0 ([`7969224`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7969224919793e6db96e6b679b8b3d688ee5768e))

### Features

- DNP3 (IEEE 1815) data-link frame codec (DETWS_ENABLE_DNP3) ([`5d1ded2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5d1ded28c315fef8c81cdedac45fce99f52ac282))

</details>

## [4.32.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.32.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`e55e43e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e55e43e4b1c57a7240739b8c0becfd69075f173c))
- update CHANGELOG.md [skip ci] ([`e4163e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e4163e1f36f120483eba5aad6ab34fe18e4163a1))

### Changes

- Bump version: 4.31.0 → 4.32.0 ([`7eb3fc8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7eb3fc8cb3c58edcee172f07920ba1fba27578a4))

### Features

- IEEE C37.118.2 synchrophasor frame codec (DETWS_ENABLE_C37118) ([`6d37445`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d37445913f2e6fd92ece98cb44ed6b310ad8408))

</details>

## [4.31.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.31.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`8f5c874`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f5c8744c6827a6e5fe6cc76396989d468a11679))
- update CHANGELOG.md [skip ci] ([`a579480`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a57948050c60955f2c91ca3f2221ea0b9772f5a2))

### Changes

- Bump version: 4.30.0 → 4.31.0 ([`8878863`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8878863658a2c88327034b91cfb5f464c7210768))

### Features

- SunSpec Modbus model codec (DETWS_ENABLE_SUNSPEC) ([`6807c9b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6807c9be342cdecc6e71bdfc06ba93c8b4c15f44))

</details>

## [4.30.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.30.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`76323b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/76323b3916bcd555eb06de3908e07c1955626757))
- update CHANGELOG.md [skip ci] ([`7ebf1b7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ebf1b73f79cadfe489d44f48f1fc96d8dee7de0))

### Changes

- Bump version: 4.29.0 → 4.30.0 ([`599bb9e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/599bb9e42d9207eca307ee47c2c0f111e0e7973a))

### Features

- WAMP messaging codec (DETWS_ENABLE_WAMP) ([`3345422`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/334542200d0fc23c1d0478dd0bd975f7cd99a525))

</details>

## [4.29.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.29.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`d4c4dda`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d4c4dda93a02932427c9f74e8de5cd181057e9a7))
- update CHANGELOG.md [skip ci] ([`fad4a9e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fad4a9e4e832603bc841a7d4a4ce987d94889cf4))

### Changes

- Bump version: 4.28.0 → 4.29.0 ([`8e87b2a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e87b2a89cb9ccc4e6686a5a5ca64a78b814b9e7))

### Features

- Protocol Buffers wire codec (DETWS_ENABLE_PROTOBUF) ([`dc4c9fe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc4c9fee8e17e58d4f7642e194024d95508daf92))

</details>

## [4.28.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.28.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`7e6f9f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e6f9f12ee119b73a3c6ed0ab5f6a78bc7778402))
- update CHANGELOG.md [skip ci] ([`59f8b8c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/59f8b8c2a70671476e4566a7f14fd87e4d62f646))

### Changes

- Bump version: 4.27.0 → 4.28.0 ([`3ae3291`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3ae3291bb477afb5f0b4030b90d865b5cbc0caa5))

### Features

- NetFlow v5/v9 + IPFIX exporter codec (DETWS_ENABLE_FLOW_EXPORT) ([`792bb36`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/792bb36fb99cd7376ff62f78cee8936dcef7dda2))

</details>

## [4.27.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.27.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`d06cadb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d06cadbaeca97219edd99d1cccdb370944fdff3d))
- update CHANGELOG.md [skip ci] ([`d0098dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0098dd1c6f36c2634e2f96822db667241f202df))

### Changes

- Bump version: 4.26.0 → 4.27.0 ([`4613dc1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4613dc164b1551e4f0c4e78fc229e92048db377d))

### Features

- MQTT-SN v1.2 wire codec (DETWS_ENABLE_MQTT_SN) ([`2682eff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2682eff47977ebfa15830993bf0201dd23158912))

</details>

## [4.26.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.26.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`0749764`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/074976494725f1ac730ea47a0a0f848b8f81e6e2))
- update CHANGELOG.md [skip ci] ([`ce52c22`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ce52c2247da58460f960f3fda87631669367533f))

### Changes

- Bump version: 4.25.0 → 4.26.0 ([`a8834da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8834daf9f06fa2e17c5a438492efe56ef96df41))

### Features

- STOMP 1.2 frame codec (DETWS_ENABLE_STOMP) ([`076b223`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/076b223cd14b7180b3d6ab159d696b3e9b3ed0d3))

</details>

## [4.25.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.25.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`b0cda4f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b0cda4fb44b214b66dbd7627c7e9277fb68cedbe))
- update CHANGELOG.md [skip ci] ([`f03f7d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f03f7d7fc77d7f9ed480ce6ddc04a511c42ae97c))

### Changes

- Bump version: 4.24.0 → 4.25.0 ([`833d98c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/833d98cdc7984954b2894454ca4498d2f2cbbf71))

### Features

- RESP2 wire codec (DETWS_ENABLE_REDIS) ([`7d674a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7d674a3e59d5b85032678127bb320d0c8cfbf6be))

</details>

## [4.24.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.24.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`81d74c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/81d74c1057e1281e35dec431d2599b601269d5ad))
- update CHANGELOG.md [skip ci] ([`75350fc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75350fcd8e10c12e420b279ca64c5943c5c5ee63))

### Changes

- Bump version: 4.23.0 → 4.24.0 ([`58a0c1e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/58a0c1e56a7ae4f02fa348eab0fc3b4af39dfedb))

### Features

- full InfluxDB line protocol - tags + timestamp ([`e013750`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e013750722bfc4c0f8cfcf24e062305e3468ee70))

</details>

## [4.23.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.23.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`388b78f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/388b78f45f966e0e45cda9bf7ee806dc26cf883f))
- update CHANGELOG.md [skip ci] ([`21ed634`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/21ed6349d33c94ba12c6f62526edec1543b6a1a9))

### Changes

- Bump version: 4.22.0 → 4.23.0 ([`4fa831c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4fa831cf0725210fac4d7fd2ea6e07731b764630))

### Features

- CloudEvents v1.0 envelope (DETWS_ENABLE_CLOUDEVENTS) ([`5c41111`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c411117895e8bbd2f1a29ac633e6c63ea233e84))

</details>

## [4.22.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.22.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`4635a8b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4635a8b9a612ceea07d9bc55d217c5a9c4bfaf14))
- update CHANGELOG.md [skip ci] ([`8b2168a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b2168a674c438d5a043d7555c924011ae62d4af))
- update test report [skip ci] ([`c18e970`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c18e9707dbfc619d013a033a30763186d83a6a76))
- update CHANGELOG.md [skip ci] ([`b757454`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b7574549ef7bab62ef7602a5196dccc381b1837c))

### Changes

- Bump version: 4.21.0 → 4.22.0 ([`d863a05`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d863a0537618d29fb8c2a1d5d4066dca0b8f4b78))

### Documentation

- keep Forwarded ROADMAP code span on one line for prettier ([`3b3d0ea`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b3d0ea64f9b506daf6ec389a93539028a948ef0))

### Features

- RTU serial framing (DETWS_ENABLE_MODBUS_RTU) ([`b8b20e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b8b20e1e11e74d25f7931d3d8fa407e27bc1254a))

</details>

## [4.21.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.21.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`903db92`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/903db922e26e9a8c769fbcd7060df38313058f65))
- update CHANGELOG.md [skip ci] ([`fb0eb3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb0eb3fd12353eb556aaeae1e7f4ba1b0c89874c))

### Changes

- Bump version: 4.20.0 → 4.21.0 ([`88c4fe0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/88c4fe0ef50fb2de7ea639f856503979fc5d55c5))

### Features

- Forwarded / X-Forwarded-For parser (RFC 7239) ([`d7c4e3c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d7c4e3cf4fd74edfb71f8ad365318c93a16c02a2))

</details>

## [4.20.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.20.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`f686439`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f6864397e71960ad5c1a9305ee658661f5603942))
- update CHANGELOG.md [skip ci] ([`d154203`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d154203d23d64d6abe5ed73996083efa2887efab))

### Changes

- Bump version: 4.19.0 → 4.20.0 ([`6fdcce2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6fdcce2ca71d30df7b60ab0b44949cf163790bd8))

### Features

- inbound Cookie parsing (RFC 6265) - http_get_cookie ([`a69bb5a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a69bb5aa1cae3787b55d7cbe53ec843853848ddf))

</details>

## [4.19.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.19.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`67019b0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/67019b09e5b92ed95650246e25b5a7a418aed723))
- update CHANGELOG.md [skip ci] ([`f71b8be`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f71b8be92968d228ac5562e7e9bc66c95318655d))

### Changes

- Bump version: 4.18.0 → 4.19.0 ([`3be074f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3be074f3189eaed70f1e323cb2921d3e45bee6c4))

### Features

- RFC 4253 7.2 key-derivation length extension ([`7a5d60f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a5d60f3e65397b78727a3908859f8caf37753bb))

</details>

## [4.18.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.18.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`9bed8fe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9bed8fe26f88e5e3f6f27f8458de9d2e903b6294))
- update CHANGELOG.md [skip ci] ([`4118445`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/41184458f22954e36ac2d8d905bd6ca03b501b52))

### Changes

- Bump version: 4.17.0 → 4.18.0 ([`8119fdc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8119fdc604ef592ffb041d4245260e790da7c68e))

### Features

- client-side session resumption (csess) ([`5145ec9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5145ec9eb8b2f5ee67dda81f16f386355f8d2d6f))

</details>

## [4.17.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.17.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`8360b4a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8360b4a838bb5f22ec2f28c71aa95188e25426c7))
- update CHANGELOG.md [skip ci] ([`bdd7c80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bdd7c805e69c2a28a0202016daf0a0b78e72faf9))

### Changes

- Bump version: 4.16.0 → 4.17.0 ([`5a48dc8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a48dc88c0b4a8f365ba273773ca7e82fc1b6759))

### Features

- optional Date response header (DETWS_HTTP_EMIT_DATE) ([`c65471b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c65471b3559490a954ba719a0eb21c62c5a5bea4))

</details>

## [4.16.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.16.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`9fce9bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9fce9bf31ecfa03495e1eaa083c4b31cef83f32d))
- update CHANGELOG.md [skip ci] ([`80303a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/80303a7621f3370a88b75c7a591f6995cf97e699))

### Changes

- Bump version: 4.15.0 → 4.16.0 ([`25a007b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25a007b14a863a5130803801189513e5bae8a6e5))

### Features

- RFC 6690 /.well-known/core resource discovery ([`e0fb976`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e0fb976571ac0bc9ffa28788c48a919ace672199))

</details>

## [4.15.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.15.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`54112a6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/54112a6249e6259bb6eed90dad5e27d834b9ead3))
- update CHANGELOG.md [skip ci] ([`5422af4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5422af43eb7b3afba15bc879956c8eb02f426f1b))
- update test report [skip ci] ([`5ed99af`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ed99aff51804abea275999493f2206a6b997d31))
- update CHANGELOG.md [skip ci] ([`93422ff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93422ff1db7497330615cab40ec8ecb0b6ecd546))
- update test report [skip ci] ([`99582ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99582ab72dd2cd4714f98b428fc1c489745f1638))
- update CHANGELOG.md [skip ci] ([`0bd20c3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0bd20c3a989c4538b6d4e8769fb89f68178feaed))
- update test report [skip ci] ([`9a3e17b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9a3e17bd3b37e03a64329154d747db613c661d4f))
- update CHANGELOG.md [skip ci] ([`e8636fc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e8636fc795f03fda8c42555cd5c63b808b36a295))
- update test report [skip ci] ([`bd7277b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bd7277b8cf9ba6d159bbcb51a5a04529c304630e))
- update CHANGELOG.md [skip ci] ([`8a3b216`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a3b216aa00e1876fa82726afb2af8586027c4f3))

### Changes

- Bump version: 4.14.0 → 4.15.0 ([`eee12c3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eee12c3438eaa23bb4a4f4d633240fdb352b1dce))
- clang-format the WebDAV handler test + FS mock ([`0a1d1d8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a1d1d8cc52bd3b613e7a3a43ac5a043c7e5ae86))

### Features

- SNMPv3 USM InformRequest (snmp_inform_v3) ([`bf30a20`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf30a201b40167595fb68bb9713e0dc882df6db3))

### Testing

- host tests for OPTIONS, GET-through-mount, LOCK/UNLOCK ([`1371644`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/137164412d03795a61ff9e309979aad9eea475f9))
- host tests for PROPFIND Depth 0/1, MKCOL, single DELETE ([`713b3ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/713b3ce30bfbcbd9878f575fbe8a14da87199d4e))
- host coverage for recursive COPY/MOVE/DELETE ([`89cd53b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89cd53b06e914a865878becd97eb4726beb3c233))

</details>

## [4.14.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.14.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`fa0daf6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa0daf60aaaf407c91734b939011391f2d884635))
- update CHANGELOG.md [skip ci] ([`51bacc2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51bacc28b1ebd8388446106c3e3bf3a3c5c354b5))
- guard the interop harness (compile + list peers) ([`ba9dca5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba9dca5e1e140eabdd0ae4b6aa0b280420f5f179))
- update test report [skip ci] ([`5ddf350`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ddf3508fe26c603c6fb5bed4b059a70afb77150))
- update CHANGELOG.md [skip ci] ([`ad4e850`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ad4e850ddaa3b87a8510dcad2038410fb372442e))

### Changes

- Bump version: 4.13.3 → 4.14.0 ([`502b0f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/502b0f6de1ecebd5b1413bf644064f6131b4968a))

### Features

- recursive collection COPY (RFC 4918 9.8) ([`af6cd33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/af6cd332217a66395e2103af9e8677e5b562a396))

</details>

## [4.13.3] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.13.3 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`b5b329c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b5b329c7c84922443a126feb9345174f0ac03632))
- update CHANGELOG.md [skip ci] ([`ae506ca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae506caf7f06a7d1fd2e159cb60d82775ec1d1ac))

### Changes

- Bump version: 4.13.2 → 4.13.3 ([`617f8eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/617f8ebd29660988b5f7ab78df6664dc62e81d19))

### Testing

- OPC UA HW-verified (7/7 families); fix stale OPC UA docs ([`ab0feb6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ab0feb65f6c437ced69069dfe0c1fdec1341eec7))

</details>

## [4.13.2] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.13.2 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`a11beb0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a11beb03d1fa41ad2456c1cd3d826f2dacfd8b00))
- update CHANGELOG.md [skip ci] ([`34eec1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/34eec1cefdcd257aae3e67fbaf80db484e48b798))

### Changes

- Bump version: 4.13.1 → 4.13.2 ([`3b83417`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b83417378a0bc8d7b9f11e1efb98b08c07e2ffe))

### Testing

- amqtt mqtt-broker fallback; MQTT HW-verified ([`81703dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/81703ddbc456c9d9b176d78907a44ab1720eed3f))

</details>

## [4.13.1] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.13.1 - 2026-06-29</b></summary>

### Bug Fixes

- compile det_client only when a client transport is enabled ([`87b94f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/87b94f1010e27af2f49aaf3d687938a28c9ea316))

### CI / Build

- update test report [skip ci] ([`d082813`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0828130f1556c1fac9dc5b89f698ff36ef90e58))
- update CHANGELOG.md [skip ci] ([`635e1e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/635e1e9cb8c98d73b8de07d205c03b9e17c1fc8a))
- update test report [skip ci] ([`e600342`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e600342f64f643f024bfbbed4bb4efade42355a5))
- update CHANGELOG.md [skip ci] ([`b4ebab4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4ebab42584fd59218d5edfc7d2b1ddec1a0dff9))

### Changes

- Bump version: 4.13.0 → 4.13.1 ([`0223ea2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0223ea21b2f296954c5d9b4fa00f1b15e5a43d8b))

### Documentation

- wrap interop CLI example so prettier accepts the code span ([`d136dfa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d136dfa1cfe2c59e7bedf3ce0105251954650ab3))

</details>

## [4.13.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.13.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`2a3c936`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2a3c936638b5c58c01d8528bbe0a6966a3c770f4))
- update CHANGELOG.md [skip ci] ([`ae4c869`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae4c8693cff8ef41c6771f6ffb59e230ea2fa2fe))

### Changes

- Bump version: 4.12.0 → 4.13.0 ([`c3c04fd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c3c04fdc36dd496adb95acac06a93721bbbbfa49))

### Features

- real-protocol interop harness (test/servers) ([`2ff7086`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ff7086d447ccbb36fe7a359b11c5cd594bcf191))

</details>

## [4.12.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.12.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`0840d13`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0840d135814cba0608467dced73c119bba7b76bf))
- update CHANGELOG.md [skip ci] ([`5ecc610`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ecc61049d622dc84787ba47b8259f3648b197aa))

### Changes

- Bump version: 4.11.7 → 4.12.0 ([`39efea5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/39efea50e52790eda4dfe3a1e0ac7613823429bd))

### Features

- close remaining standards-audit LOW/SHOULD items ([`87ba5a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/87ba5a74e0ff09626a466c5f6328ef2684ec9d03))

</details>

## [4.11.7] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.7 - 2026-06-29</b></summary>

### Bug Fixes

- reject PROPFIND Depth: infinity with 403 propfind-finite-depth (audit batch 2d) ([`71b8b32`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71b8b328c0d5865d68bc6e7bda8c542cca89f3ff))

### CI / Build

- update test report [skip ci] ([`8b99388`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b9938831e4585b981a6da932293dc1f87aa965f))
- update CHANGELOG.md [skip ci] ([`4ff4dbc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ff4dbc5a162d66ec500195cffc13393f797a7b0))

### Changes

- Bump version: 4.11.6 → 4.11.7 ([`cf56025`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf56025c45fb0072b267f958c4908caf1f49fc85))

</details>

## [4.11.6] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.6 - 2026-06-29</b></summary>

### Bug Fixes

- If-None-Match weak comparison + list + "*" (RFC 9110 13.1.2) (audit batch 2c) ([`ae1e131`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae1e1313e768461a135f7375bc73ec16029918fc))

### CI / Build

- update test report [skip ci] ([`c7b91f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7b91f182cf562a731f4dac4fa3952b18639bfad))
- update CHANGELOG.md [skip ci] ([`f002bb9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f002bb96c2802dbea0308fe2254d5de8d300366a))

### Changes

- Bump version: 4.11.5 → 4.11.6 ([`a08ef33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a08ef3368ff1a4933b8265d1b309899026a8d781))

</details>

## [4.11.5] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.5 - 2026-06-29</b></summary>

### Bug Fixes

- WS text UTF-8 (1007), SSH padding>=4, syslog PRI clamp, BER OID first-subid (audit batch 2b) ([`90267f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90267f6bf8195f8c3abd47986e9c4dd80936eda5))

### CI / Build

- update test report [skip ci] ([`d64c77f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d64c77f3025232aecb37dd5e94b749fc9cc47a2b))
- update CHANGELOG.md [skip ci] ([`5a36a7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a36a7f649c2586ac46a903129b73b5ad6987958))

### Changes

- Bump version: 4.11.4 → 4.11.5 ([`a0c4c36`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0c4c3645b699f876e290151baf2163bcd258eb5))

</details>

## [4.11.4] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.4 - 2026-06-29</b></summary>

### Bug Fixes

- validate JWT alg header + match Digest uri to request target (audit batch 2a) ([`2c6f0c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c6f0c10158ec881d0e1dbd56c4e1758b20ad6ba))

### CI / Build

- update test report [skip ci] ([`6de236b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6de236b3a158a8ce57aa9bdc8e3329547bb73ee9))
- update CHANGELOG.md [skip ci] ([`b75d0db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b75d0db98294ff734f16bbe1aba2672759bd05fe))

### Changes

- Bump version: 4.11.3 → 4.11.4 ([`952ea51`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/952ea513c2b2573de74d1884f40498186dca6c67))

</details>

## [4.11.3] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.3 - 2026-06-29</b></summary>

### Bug Fixes

- WS close, MQTT QoS=3, CoAP method/option codes, Telnet IAC (audit batch 1) ([`a45da5f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a45da5f22d5c4fc4f8cdc9f8d9bb379ad334fb72))

### CI / Build

- update test report [skip ci] ([`452137e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/452137e3be950d6a419cdae90345502dcdefecbf))
- update CHANGELOG.md [skip ci] ([`854a80f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/854a80f52af4906527c5b7b68fe5df4a72de2e34))
- update CHANGELOG.md [skip ci] ([`ed56bb6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed56bb650f0f969d3d3343934b8698098512a9c6))
- update test report [skip ci] ([`c267b27`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c267b276e20af44e66963e7d7789257565f38ca9))
- update CHANGELOG.md [skip ci] ([`d0d9288`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0d9288f31eecee76d9494bed76faa9433b07a78))
- update test report [skip ci] ([`90077e6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90077e63321b7f4f8cc1a8c1b2a9036bcb563bfc))
- update CHANGELOG.md [skip ci] ([`80463fb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/80463fb724f937b247859b7e50abb8614215c4a9))

### Changes

- Bump version: 4.11.2 → 4.11.3 ([`a313038`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a313038b8d7892d6a6788815bacb2ff6048e25c6))

### Documentation

- link the learn series + STANDARDS.md from the docs landing page ([`c4f0222`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4f02223894ef0a719ea416041cb5c4c96faee0d))
- beginner from-scratch primers (OSI model, TCP/IP, languages) ([`0ceadac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ceadacd98d99b5b6a20abf917c1e188cad05e76))
- add STANDARDS.md (links every standard the lib uses) + roadmap 'audit against standards' item ([`06db79f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06db79f0e8e7426866ad0ad03398d548ccd2278d))

</details>

## [4.11.2] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.2 - 2026-06-29</b></summary>

### Bug Fixes

- Observe used millis() (host-unbuildable + pluggable-clock violation) + CI coverage for gated features ([`4333bc8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4333bc8036e3fd2a57bb8bc29256086840960e2d))

### CI / Build

- update test report [skip ci] ([`3c3e0ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c3e0cea03c3233cfac6ba6ca4aa6a2c52406dd4))
- update CHANGELOG.md [skip ci] ([`65b9a26`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/65b9a269a638462d7ad4b1a81404ac053aed2b3c))

### Changes

- Bump version: 4.11.1 → 4.11.2 ([`7e4b710`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e4b710fca4175f842daf66c08240f9b65cdec83))

</details>

## [4.11.1] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.1 - 2026-06-29</b></summary>

### Bug Fixes

- reject Transfer-Encoding on inbound requests (request-smuggling) + test-gap hardening ([`f45eafb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f45eafb40bbe716963329266ec9b689060b4298f))

### CI / Build

- update test report [skip ci] ([`7a77905`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a779052e3dee71f37b4d2151894e51534946c63))
- update CHANGELOG.md [skip ci] ([`d14e248`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d14e248e63c98f730412780fc8b44804c9bf6368))
- update test report [skip ci] ([`6b953f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6b953f778d1e4044c631d0175137b79ed3783855))
- update CHANGELOG.md [skip ci] ([`2614b1b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2614b1b411036d06215921f961f94e49c56095c2))
- update test report [skip ci] ([`80e7f9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/80e7f9c4166474ad7dd9f65fdecb18d3333d232c))
- update CHANGELOG.md [skip ci] ([`10a406a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/10a406a732f451222b31ab5c7f8ee1aaf9ca3c0c))
- update test report [skip ci] ([`9a43b18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9a43b1889c98bdb990c842c1d743fd53998477eb))
- update CHANGELOG.md [skip ci] ([`4dd142e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4dd142eb79d88c3ea43fbc043ec5b316567cc3bd))
- update CHANGELOG.md [skip ci] ([`50fcafc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50fcafcc986e9bb45584912a2b7aa6560a484729))
- update test report [skip ci] ([`1c5e46e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1c5e46e8a2f50e14b24c047bfef411d400d1e96e))
- update CHANGELOG.md [skip ci] ([`04c4d39`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04c4d3993aba64181fabb53be273a722fd98f888))

### Changes

- Bump version: 4.11.0 → 4.11.1 ([`2f284ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2f284ab4700753893f6d74c844620abdd5d53d8b))

### Documentation

- standing front-end assumption (SPI/UART/I2C adapters + ADC for 4-20mA) so no protocol is hardware-blocked ([`235f669`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/235f66930171d9de5f94b3b1a90b7b4b407849cb))
- raw-L2 enabler + real-time timing model; ESC/motor protocols; real-protocol interop harness ([`91fd148`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91fd14801030032a334ce00c371f9e53f8650592))
- add IoT/messaging/DB protocols (LwM2M, STOMP, gRPC/Protobuf, DDS, WAMP, CloudEvents, MQTT-SN, NetFlow/IPFIX, BACnet/IP+SC, XMPP-IoT, InfluxDB line, Redis/NoSQL) ([`8430840`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8430840755a2ed86fa99813dc780322c4ed424b1))
- add ITS / V2X / traffic-cabinet protocols (NTCIP 1202/1203/1211, UTMC, OCIT, SAE J2735, IEEE 1609 WAVE, NEMA TS 2, ATC) ([`1a12665`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a12665d240199fead0606d0f1b23c83c29d2883))
- add DER / smart-grid protocols (IEEE 2030.5/SEP2, OpenADR, SunSpec Modbus, ICCP/TASE.2) ([`89973e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89973e5d72f289238e4a2db3475c5075b133b29e))
- add power-grid SCADA protocols (IEC 60870-5-101/104, IEC 61850 MMS/GOOSE, IEEE C37.118) ([`ccb5f5a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ccb5f5a987ebc033b150e22ab1520390ad49d3b2))

</details>

## [4.11.0] - 2026-06-29

<details>
<summary><b>Show Changelog for version 4.11.0 - 2026-06-29</b></summary>

### CI / Build

- update test report [skip ci] ([`608abb4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/608abb4498cea660a8939eca32512fafaefe1547))
- update CHANGELOG.md [skip ci] ([`43476ad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/43476ad288c12f461203f27a1973b3a5c98f002e))
- update test report [skip ci] ([`7eab22b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7eab22b89c3a482880aa927ac9201ed6b67e1889))
- update CHANGELOG.md [skip ci] ([`0d572c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d572c94d6af6580f3a2baa63802425b29953fca))

### Changes

- Bump version: 4.10.0 → 4.11.0 ([`b01b560`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b01b56036e5893a86ce881cd1fc3813c8bf88051))

### Documentation

- fix BUGS.md nested-list formatting (prettier) ([`489137f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/489137fc69fd08c9f664eefbb995de0f3debadfb))

### Refactor

- drop redundant pcb threading from the egress API (ingress/egress symmetry) ([`cfea4c4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cfea4c4a566b32bb88234ef0d902b9ff758f4f41))

</details>

## [4.10.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.10.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`4701a90`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4701a909ee8c209829a748cbb11c58e3905eb9a4))
- update CHANGELOG.md [skip ci] ([`a2998b6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2998b6efaeff15e1596f773e1a28bce8003713a))

### Changes

- Bump version: 4.9.2 → 4.10.0 ([`3181bb3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3181bb36352a205e6200488345e9a16ec282fd63))

### Refactor

- shared response helpers, MIME/byte-cursor primitives, unified base64url + DNS ([`817ca53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/817ca53b097c8c90308e9824744fcdab572835d5))

</details>

## [4.9.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.9.2 - 2026-06-28</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`d4d314c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d4d314c8eecb1cd3caef4d6b0cab964fa1641a36))
- remove editor-tool ignore entry + stale changelog line ([`8e4d25e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e4d25eb808786077979da8942324eb3305b133a))

### Changes

- Bump version: 4.9.1 → 4.9.2 ([`ab2e6ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ab2e6ed0780ce277a0dfed28df8c3f961358cc10))

### Refactor

- shared det_hex + relocate primitives to src/shared_primitives ([`c1a8669`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c1a8669774024124beecc7a3ffb4592fdce93eae))

</details>

## [4.9.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.9.1 - 2026-06-28</b></summary>

### Bug Fixes

- range overflow, If-Modified-Since month mis-parse, dns_resolver clock ([`57675c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/57675c05d675f06bb5dcffb75dbb147f1eaaa580))

### CI / Build

- update test report [skip ci] ([`2bc855e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2bc855eb3646c006fc88ffc9953a7a3c60ec696f))
- update CHANGELOG.md [skip ci] ([`369ad94`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/369ad94b29f2450ae7e844d6a0ca02133b43e03a))

### Changes

- Bump version: 4.9.0 → 4.9.1 ([`5697ef1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5697ef1ffb7292187b9dfd821d68315936d20cef))

</details>

## [4.9.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.9.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`0402eb8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0402eb8dbff6b1bde79c513d67792e3dd6a8695a))
- update CHANGELOG.md [skip ci] ([`9b42ce7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b42ce7113f6e194ea427e40890c269f53f7ad4e))

### Changes

- Bump version: 4.8.2 → 4.9.0 ([`2b95cbb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2b95cbb9ef0f64648a00959aa344e30db83d4aa8))

### Features

- Last-Modified + If-Modified-Since conditional GET on served files ([`715c5da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/715c5da3535494b45a3ff5372ddf9921ad65f04c))

</details>

## [4.8.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.8.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`5d56f51`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5d56f5149e151abeb54685ec0ae82a6ef9ced510))
- update CHANGELOG.md [skip ci] ([`9508f7a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9508f7ad3e9a6459eaef0a717bfb941e8d421069))

### Changes

- Bump version: 4.8.1 → 4.8.2 ([`4b202bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b202bc21d4fa9c04a442f35cb50cf4d591c23ec))

### Performance

- share the ring producer too (bulk memcpy both sides) ([`75f77bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75f77bfdea0d70c03656cf9202ebce6f5e5a94d6))

</details>

## [4.8.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.8.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`946d971`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/946d97136db2c9b7244c9f536cb4bd3cda5d8d88))
- update CHANGELOG.md [skip ci] ([`9f5df31`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f5df31b7e1032cb43467a4c4b5f39c3c25c18bf))
- update test report [skip ci] ([`bb04311`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb04311a5ca62d952f1b45719c04aa81a8b7b225))
- update CHANGELOG.md [skip ci] ([`7ea66b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ea66b16f47c7da483cefe491931ef41f8ca53a3))
- update test report [skip ci] ([`a79e38b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a79e38b4dfbfcccdacf529dc14b1b2bd6d096f84))
- update CHANGELOG.md [skip ci] ([`ccf92bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ccf92bccd1b3e7a2bba2bf4e3a7446c50c10589f))

### Changes

- Bump version: 4.8.0 → 4.8.1 ([`bebffe0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bebffe003327443aedb12f2497dfc3ff5e9f5b5b))
- clang-format hook decl + cspell add doc/test words (fix CI) ([`5177f78`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5177f780748c391bce9f2ebb9a83e8b12004369e))

### Documentation

- record HW validation of the client-transport ack-on-consume fix ([`14fbadc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14fbadc1a8b3b5392f0b1ce0a4908879ef83313e))

### Refactor

- one shared SPSC ring primitive for both transports ([`7edc28c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7edc28c5287c30724f7d53e8078bd301550abfe7))

</details>

## [4.8.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.8.0 - 2026-06-28</b></summary>

### Bug Fixes

- ack-on-consume on the outbound transport (det_client) ([`deb71d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/deb71d9f8eb740f262858c019625633eeabd6961))

### CI / Build

- update test report [skip ci] ([`f36066c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f36066c5251a0d8f570defffa88b4a6ddd564193))
- update CHANGELOG.md [skip ci] ([`9d648a9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9d648a92521d85e82b14e071bcdcc10b93a2bb5f))
- update CHANGELOG.md [skip ci] ([`5896ed3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5896ed31bcfda9fa18d26b4ac12253c341fd4c56))

### Changes

- Bump version: 4.7.0 → 4.8.0 ([`ee2a89f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee2a89fb4393a017732b8cea8b3fc0e4f629e8e7))

### Documentation

- close internal-piping Phase 4 (client buffers documented) ([`a036811`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a03681158d3477a335961ae88678c36f7afe85e6))

</details>

## [4.7.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.7.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`d0df4d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0df4d924bb69a2ec95da131e45b997ddf9a3dc5))
- update CHANGELOG.md [skip ci] ([`b5f3892`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b5f389281d99f819ba3905e7612db383287ca865))

### Changes

- Bump version: 4.6.1 → 4.7.0 ([`bdb56cd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bdb56cdd87c06573274b775e32cb1dbfc2d12db0))

### Features

- concurrent streamed PUTs via slot-aware streaming hooks ([`17cb428`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/17cb42863fd482c8bc0ba99d1b938c22ee302ff2))

</details>

## [4.6.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.6.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`430801e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/430801eab0061e62d095113210104058adffb389))
- update CHANGELOG.md [skip ci] ([`51402ab`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51402abb1272288cceb573e7a844bbdcc66d4dcd))
- ignore .pio deps (Unity) so third-party findings stop recurring ([`ba52844`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba5284411cc78dbe00b54360a5154d1b3d6ce696))
- update test report [skip ci] ([`513a523`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/513a52320f3d933d6a51193bf80b21ae5222706d))
- update CHANGELOG.md [skip ci] ([`5ee2d05`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ee2d05c7e418f0710084007ceb1f698e62b4b5d))
- scrub from library.json; gitignore / ([`d04cb80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d04cb800651ac0fd0da3889a5c9a11f52be0cd5b))
- update test report [skip ci] ([`c69233e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c69233e4491540d472260ecca250578ff7a2fc7b))
- update CHANGELOG.md [skip ci] ([`b547143`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b54714344d70c69db3590841e9334a766a8aa477))

### Changes

- Bump version: 4.6.0 → 4.6.1 ([`6fa4ed6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6fa4ed617f11fa731010e972c39897154b1eb57c))

### Refactor

- single RX read API; consumers stop poking the ring ([`2c5d0ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c5d0bad560f63aad096155ded39e45562a39e20))

</details>

## [4.6.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.6.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`394eee9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/394eee92cfc5a53e532d8af2b077c2f992713111))
- update CHANGELOG.md [skip ci] ([`400794e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/400794eac8809b3eebccc33f0bdec2dd77c5a68b))

### Changes

- Bump version: 4.5.0 → 4.6.0 ([`f5cff73`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f5cff73e5b2542151077dc04e21b8ec8cf2e5293))

### Documentation

- roadmap TLS 1.2/1.3 + HTTP/2/3 (RFCs); README not-audited notice ([`b9b6362`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b9b6362ce03d235e0972e539001c54d54e034ec2))

### Features

- stream PUT bodies to disk; make RX flow control deadlock-proof ([`5e1bcd1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e1bcd1af59c979c857a801c3c7bfc87efcfe0f1))

</details>

## [4.5.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.5.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`08af379`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08af379410f9dc2bae71788a65a0bb27094639d1))
- update CHANGELOG.md [skip ci] ([`11d151b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/11d151b2070bb92e92bb486baa46b944d0dc0742))

### Changes

- Bump version: 4.4.2 → 4.5.0 ([`e348e1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e348e1dad434cff7539ed9eda2a08fc57e78ee08))

### Documentation

- expand the educators note; sync README -> docs/README ([`4ffed17`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ffed17c6424072e20adad36c71e1b4575c753e1))

### Refactor

- pull-generator send_chunked, paged across loops (no truncation) ([`8ef4c6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8ef4c6c6e0d1bcaf325f234f937ffd971ce81a81))

</details>

## [4.4.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.4.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`37ba052`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/37ba05243dcfdd52af0667f3d34c3962330ef10d))
- update CHANGELOG.md [skip ci] ([`562418c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/562418c67a0ee4f664142e347c8ef8a3ce49f3fa))

### Changes

- Bump version: 4.4.1 → 4.4.2 ([`3c01e80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c01e807cc06c6e9f9812e69f2b40cf00397896c))

### Documentation

- dual-license section (AGPLv3 always-open + commercial + educators) ([`ba5767b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba5767bbe70c4f6aaf4c06023e8d219699d0549c))

</details>

## [4.4.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.4.1 - 2026-06-28</b></summary>

### Bug Fixes

- page large file responses across loops, never truncate ([`48ce846`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/48ce84606ca8630a3bb2cf117d5a8e1e8ff3ed35))

### CI / Build

- update test report [skip ci] ([`e378da3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e378da30cc7f6f9805a3278a6141baee2328b3e8))
- update CHANGELOG.md [skip ci] ([`30d97b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30d97b46cf4df01e30567233b72cfac7079cb18f))
- bump actions/checkout from 4 to 7 ([`3a71dc5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a71dc53d92ba8e609117d9f41ac5384db933a8b))
- bump github/codeql-action from 3 to 4 ([`3f0e462`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f0e462cafe2f0df76d0b60f5ccbf93c8f34699c))
- update test report [skip ci] ([`ef3a48b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef3a48bbfb7e7f0c47fc711b2193fc32bb19ed15))
- update CHANGELOG.md [skip ci] ([`5987236`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/598723690af035597488a4a705e8fea8b9cde80d))

### Changes

- Bump version: 4.4.0 → 4.4.1 ([`9f67ab0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f67ab06013bedf29d24eca62e4c0f49f114a50b))
- Merge pull request #4 from dstroy0/dependabot/github_actions/actions/checkout-7 ([`62593c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/62593c9df205fc138aecbff6fcc5bbb468bfcb98))
- Merge pull request #5 from dstroy0/dependabot/github_actions/github/codeql-action-4 ([`d41f6e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d41f6e07f618a7615072b601ace86cb010a04059))

### Documentation

- add a prominent active-development / breaking-changes notice ([`af68d81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/af68d8159faa5935e023677e1559709a9577d118))

</details>

## [4.4.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.4.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`28346fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/28346fa2a54e05b70ee89535ae90b3796148f070))
- update CHANGELOG.md [skip ci] ([`4612b53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4612b53b089e49dbc937863d423074e01b46be67))

### Changes

- Bump version: 4.3.0 → 4.4.0 ([`0920424`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0920424b22f709d8777e5b2aa5a83a13c82e47bf))

### Features

- answer PROPPATCH with 207 Multi-Status instead of 405 ([`2235e72`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2235e722ca21d1ec5b6ec08b46e667f6676a1bce))

</details>

## [4.3.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.3.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`6c65228`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c6522876a87a5f59a2d277a2b068f9d7a9747c8))
- update CHANGELOG.md [skip ci] ([`82355c1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82355c11933975f93bf6649396ae127955c6197f))

### Changes

- Bump version: 4.2.0 → 4.3.0 ([`53b3141`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/53b3141f5a811e74c029ddf2345d8d167ec38db1))

### Features

- add a zero-heap MessagePack decoder ([`f43e281`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f43e2812fb32bb5b096c0952129a35fe167ed52d))

</details>

## [4.2.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.2.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`6e9d3c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6e9d3c80b57e4008b9ac9cbb377dee8b563ecfa5))
- update CHANGELOG.md [skip ci] ([`778cff5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/778cff59bfc8e6b5a3632881a965418df543f5d7))

### Changes

- Bump version: 4.1.1 → 4.2.0 ([`74d0f5d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/74d0f5d4a42b8dbf6324244e51289c558d1c264e))

### Documentation

- add TUNING.md worker/perf guide; mark concurrency tuning shipped ([`c041c00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c041c0008ee4b9e7f5dd779ab09987ddb9bcdfa1))

### Performance

- bulk-copy received segments into the ring with a single publish ([`1bbd8e5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1bbd8e5aed6de00c7f7576a35d7c7a59c8e6c368))

</details>

## [4.1.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.1.1 - 2026-06-28</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`5bf07e7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5bf07e7f3e1c6426599de29c1147318b49eb6d2f))

### Changes

- Bump version: 4.1.0 → 4.1.1 ([`e88906e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e88906ea3f04edded5acd307fabfebd1300354e5))

### Documentation

- mark notification-driven worker drain shipped ([`9aa113f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9aa113f3d9c943938e9e97196a538307870ab402))

</details>

## [4.1.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.1.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`f7d6f12`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f7d6f121b2e479932d0c1df5800a4a05c42293f9))
- update CHANGELOG.md [skip ci] ([`1ca1ded`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ca1dedd989fe4cf58d20989566c7198e266350d))

### Changes

- Bump version: 4.0.0 → 4.1.0 ([`2ad2bd9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ad2bd99e3fdaed9f131e1b67cd746245b765a04))

### Performance

- notification-driven blocking drain instead of the fixed poll ([`2ace854`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ace85490dcc6ba3e77982f801ef5e28392e1450))

</details>

## [4.0.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 4.0.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`9cfd7e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9cfd7e8fd33002cd9760bcfbf5e303f5cac00b51))
- update CHANGELOG.md [skip ci] ([`9781b50`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9781b509f3b953d1bba102d37ccdc41ae553cbba))

### Changes

- Bump version: 3.14.0 → 4.0.0 ([`4e79070`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e79070bd30893e07a9ea85e9f6be7fbe2ac183f))

### Refactor

- remove heap_needed/heap_available shims and the PROTO_NONE HTTP fallback ([`455ab53`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/455ab53a03347290005d1cd024f7f36f93014fa9))
- fold the 12 codecs into per-component subfolders ([`866f421`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/866f4212672854a47b03628316b80b339196aed8))

</details>

## [3.14.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.14.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`e9f6c7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e9f6c7ffd84cd8ef80620cd06f9c991191b179fc))
- update CHANGELOG.md [skip ci] ([`00a66da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00a66da3176b2ba00444136b3016b18a8ac5379f))

### Changes

- Bump version: 3.13.0 → 3.14.0 ([`d54d72d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d54d72de4966b975e10d79978b5ebed563e33050))

### Features

- migrate ws_client onto det_client (Bucket 3b, 3/3) ([`9cba440`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9cba440d3a63d923ac13d36e157a82f74c067982))

</details>

## [3.13.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.13.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`7703a29`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7703a29ce4b5c50be0894eba1ee33a7ec887d254))
- update CHANGELOG.md [skip ci] ([`5e6f3f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e6f3f6183ead2d23cd8eb407cdf741d25797f98))

### Changes

- Bump version: 3.12.0 → 3.13.0 ([`53099e7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/53099e7a75eb4706905205c950423ffb46f60194))

### Features

- migrate mqtt onto det_client (Bucket 3b, 2/3) ([`2181a17`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2181a17de55c1fb2f79043a891863b1e26a9a6c9))

</details>

## [3.12.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.12.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`a97d108`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a97d10807514f2b34a107f0e337a4b2619660895))
- update CHANGELOG.md [skip ci] ([`d43dd00`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d43dd004e4b15bcd3310fc12cb3d61b6657e0ada))

### Changes

- Bump version: 3.11.0 → 3.12.0 ([`2063f6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2063f6ea7e31dd52d895f3a67f49999fd061a58d))

### Features

- shared outbound client API; migrate http_client (Bucket 3b, 1/3) ([`2411506`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24115062bf6721195c52b2a2a04f7b37d44bb0b6))

</details>

## [3.11.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.11.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`7efdbc7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7efdbc751646859cef0c382766672d291e721835))
- update CHANGELOG.md [skip ci] ([`5ada2a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ada2a3dc4b47839e27379bec35257337d7e883d))

### Changes

- Bump version: 3.10.0 → 3.11.0 ([`5bd7afa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5bd7afa93b736cb38666475f2b4dd66fd7387bcf))

### Features

- make CONN_CLOSING a real dwell (Bucket 3a part 2) ([`87b490f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/87b490f454b74b05f8a89705e63509e11861d936))

</details>

## [3.10.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.10.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`1625732`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/16257325c97a41588bf6dfb12f8a54751b1c5ee7))
- update CHANGELOG.md [skip ci] ([`134a8ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/134a8ec4212e92ed53b3b15b0253963fb76b4043))

### Changes

- Bump version: 3.9.7 → 3.10.0 ([`3c72803`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c72803856169ae2c89bfc2418284fa4b47e5b82))

### Features

- observability hook + counters (Bucket 3a, part 1) ([`12d1961`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12d1961deea2481ef988ee6d86fe9de345955bbf))

</details>

## [3.9.7] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.7 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`c450f1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c450f1fd7572f14720f19a1983723a66e07ea136))
- update CHANGELOG.md [skip ci] ([`aa6c130`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aa6c13083ae1bb187e0077c72d2f36663d196e34))

### Changes

- Bump version: 3.9.6 → 3.9.7 ([`0b89991`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b899918111efa12c67ae47244c0c7850bda14da))

### Testing

- live progress (counter + percent + spinner) in run_tests.sh ([`4e59944`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e59944bd236adc635710e467b3c8067e55ec6f8))

</details>

## [3.9.6] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.6 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`30aa45b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30aa45b298815714200637babb4d32230dc3ce76))
- update CHANGELOG.md [skip ci] ([`b906b7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b906b7ff62d5c9d1a957a853ecb8511b7e2020a1))

### Changes

- Bump version: 3.9.5 → 3.9.6 ([`c065794`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c06579497f25b1a5c49eda6ec78828afff2abff4))

### Testing

- table-driven env generator; runner auto-discovers all 60 envs ([`1314259`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1314259d7b5b0efbc6ac2a04b0e8ead43d9e25a4))

</details>

## [3.9.5] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.5 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`fb896b1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fb896b197081f3a0a44ef3f35174fa284cbcd81d))
- update CHANGELOG.md [skip ci] ([`5ecfede`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ecfedeeb1936686fb9a8130409ce15615121df9))

### Changes

- Bump version: 3.9.4 → 3.9.5 ([`5e4227e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e4227ebf2a655796abe40a549c78a5aa6a9811b))

### Testing

- expand host fuzz to 23 targets + add live adversarial driver ([`875c7e2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/875c7e2e23b84ca7a817a14d71687c265705f5da))

</details>

## [3.9.4] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.4 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`d06ba81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d06ba81423e65a582e3e355f4ccc8c91cbb301cb))
- update CHANGELOG.md [skip ci] ([`a86c692`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a86c692b9500700322b35a41030cae0c6aac76a6))
- update test report [skip ci] ([`3883a9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3883a9cf70bd20bba8caf6e2d11385c7b7b1e99b))
- update CHANGELOG.md [skip ci] ([`d0c45eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0c45eb64a41f65f56c2d91f6b348ef7ffa1623f))
- pin Prettier to 3.9.1 and reformat all markdown to match ([`0350f11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0350f1170d25aeff90d5172bca1177dd8c5ad670))
- update test report [skip ci] ([`780c57a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/780c57a4f8645e8ddcc27181ea489f3670fb121c))
- update CHANGELOG.md [skip ci] ([`7789767`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7789767071f2c2be51c7f8349a2dd48f555790f5))
- update test report [skip ci] ([`99dcd65`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99dcd65158f610ea7ca989c1205df29744775076))
- update CHANGELOG.md [skip ci] ([`f2079cd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2079cd5db4cfc9d06f0f0ba9d0d1b4a821fcb8d))
- update test report [skip ci] ([`d6aa831`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d6aa831e486c7da695f404b2490ed69e5c08451a))
- update CHANGELOG.md [skip ci] ([`e2d3202`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e2d3202118629dade70d228c1a65b853506b3633))
- update test report [skip ci] ([`0ca6b32`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ca6b32dae50652cae1b1758376f3468a41370e4))
- update CHANGELOG.md [skip ci] ([`42522da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/42522da8b18d477eb124a81c719e7dee49910269))
- update test report [skip ci] ([`5adc0e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5adc0e05c00e937b793a0b503a6765b6b895979e))
- update CHANGELOG.md [skip ci] ([`47e7588`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/47e7588d63c78c1bf7e08b6c13492b79133a3276))

### Changes

- Bump version: 3.9.3 → 3.9.4 ([`0d929c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0d929c00b10e8162bfe1369cc4af6f21a67b35f3))

### Documentation

- add annotated READMEs for L7 50-56 (Oidc, Vfs, GraphQL, EspNow, OAuth2, OpcUa, OpcUaClient) ([`3473442`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/347344233e1de0464291c0f8470ee38eb898daa8))
- add annotated READMEs for L7 45-49 (Totp, Webhook, RadioPower, DnsResolver, AuditLog) ([`b23133e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b23133ea891569179a34f09c61c5382572a7d02c))
- add annotated READMEs for L7 40-44 (Guardrails, LogBuffer, ConfigExport, ModbusScan, OtaRollback) ([`924003e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/924003e0d33a40f1db8f0f90b56c2af3bb341ffd))
- add annotated READMEs for L7 35-39 (Dashboard, NetEgress, PartitionMonitor, GpioMap, UdpTelemetry) ([`4bf18fd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4bf18fdc55276c572c2752e716778cb416a3748e))
- add annotated READMEs for L7 30-34 (Modbus, TimeFallback, DeviceUuid, Csrf, Telemetry) ([`99dfedb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99dfedb5e3d11e7966f30812807cfd260152ee68))
- add annotated READMEs for L7 25-29 (WsClient, SnmpTrap, CoapObserve, CoapBlock, WebDav) ([`0ee2184`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ee2184d3ec10f815de38faa9ce9fffacb6b7aa5))
- add annotated READMEs for L7 20-24 (Diagnostics, Prometheus, Stats, HttpClient, MqttClient) ([`3610c1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3610c1fe3ab9a6f6a8632b4225aae6ae25e35ddd))
- add annotated READMEs for L7 15-19 (mDNS, OTA, Provisioning, SNTP, Syslog) ([`264d40c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/264d40cec4ee3209b4aa85df1cd1cc87837e5614))
- add annotated READMEs for L7 11-14 (Upload, Range, CoAP, SNMP) ([`c49a0d7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c49a0d7a4af6cf4ba5e97de4e64e9a8d71fd5121))
- per-example READMEs for L7-Application 01-10 ([`d56103a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d56103a4a26e5308ad20d48d171a13e4e052f55d))
- add per-example READMEs for the L6-Presentation layer ([`0eea5e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0eea5e9da0c4c5bc2d85119b952c4d9e9f48724a))
- per-example READMEs for L4-Transport; make EXAMPLES.md links-only ([`f0c8ea8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f0c8ea8b971c8478a9970059fa09b36fef19fb5b))
- add per-example READMEs for the L5-Session layer ([`0443496`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0443496611c81a577c0b3e43472473e493bab834))
- add per-example READMEs (Foundation layer) and the examples index ([`58b3977`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/58b3977f2a405b3ebdcf8dc116736a6eb989e8ce))

</details>

## [3.9.3] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.3 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`56a540c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/56a540ccfd5c194031e17ccefacf10667930bb2b))
- update CHANGELOG.md [skip ci] ([`e30fd3f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e30fd3ffe82544b64d3f0af0ab2bbcf7185849e2))

### Changes

- Bump version: 3.9.2 → 3.9.3 ([`f8f7f71`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8f7f710bff021902dba28808008f6e156dfbc52))

### Documentation

- redraw Squirty the mascot (smooth 64-grid squid) with live expressions ([`4091d3b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4091d3bc837c81677e94304a16ef61e7c2b7ad38))

</details>

## [3.9.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`a2216c9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a2216c9460750cb72397364338d1cb0bf2b984ce))
- update CHANGELOG.md [skip ci] ([`c7b7502`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7b7502207b210fb5e7957e788f6aed0934f6780))

### Changes

- Bump version: 3.9.1 → 3.9.2 ([`2eec7ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2eec7ef6d18b2f9b17e48a3ce41f77e8ae396e35))

### Documentation

- fix incorrect and missing source doc-comments from the coverage sweep ([`1041679`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1041679e79981f40d526ae62545c98435ae16c9e))

</details>

## [3.9.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`45b9ab5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/45b9ab5adbb3710caabc42fdbd4e75a4317ea9a6))
- update CHANGELOG.md [skip ci] ([`37c11c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/37c11c031e4b79986c4e3ef37a03837d4547fd0f))

### Changes

- Bump version: 3.9.0 → 3.9.1 ([`1906e37`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1906e37f52cf818cb7db247c9df5e1d4a8e0da37))

### Refactor

- move the connection source-IP read into L4 and drop dead config ([`8a814f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a814f5745690780e8043af8532bf2914b3a3255))

</details>

## [3.9.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.9.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`ad394b5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ad394b59ad52cc1b15d313ecb07de184d59381bd))
- update CHANGELOG.md [skip ci] ([`a062109`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a062109860d48dd515c43b3a4e2b59e65b72c06b))
- add contributor/maintainer health files and npm tooling manifest ([`90e41cf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90e41cfd56b5a95334aa04eae50eb77c33026578))
- update test report [skip ci] ([`610ea2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/610ea2b8b60376da6e2833a22528d443703f0727))
- update CHANGELOG.md [skip ci] ([`4ad530a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ad530af60606f1589b14333bb32faae38f2695a))

### Changes

- Bump version: 3.8.4 → 3.9.0 ([`b1f0b41`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b1f0b41c47b8610c9d4b8e60976e07a725ec644d))

### Features

- validate build-flag dependencies and document the dependency tree ([`679590c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/679590cd3a7a776cfe542f909c300a377db80a1d))

</details>

## [3.8.4] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.4 - 2026-06-28</b></summary>

### Bug Fixes

- add missing esp_wifi.h include in EspNow; correct stale @file tags ([`8ad72b8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8ad72b86e966f9eaec280564963fb69e89bb3adb))

### CI / Build

- update test report [skip ci] ([`0374a77`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0374a7723d524f52e93a667f82bfb0706a09ddbf))
- update CHANGELOG.md [skip ci] ([`8830a04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8830a044a35bf8f1d1afc3207edd5a0514077f58))

### Changes

- Bump version: 3.8.3 → 3.8.4 ([`cbc90ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cbc90ba360ff2b33107e814ff590d7ce9eae18d0))

</details>

## [3.8.3] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.3 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`f8e6952`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8e6952a20c1aa1ebbfe2263e2a6c9fb118f59ec))
- update CHANGELOG.md [skip ci] ([`96ae1ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96ae1eeaf3a799ef0dd50307814d246e23c3adfd))

### Changes

- Bump version: 3.8.2 → 3.8.3 ([`500b9d9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/500b9d93c5b04ef283a8eec84b3b1868b201af48))

### Documentation

- fix accuracy gaps found by the documentation audit ([`2c0f2c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c0f2c2137770420abb96a01c0737b53fd2d9654))

</details>

## [3.8.2] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.2 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`1dce842`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1dce84273b19a646f223f87bfee1d1359dbc17d4))
- update CHANGELOG.md [skip ci] ([`2570153`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25701532a1a934bfb7748395d946feb761e801eb))

### Changes

- Bump version: 3.8.1 → 3.8.2 ([`0a018d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a018d01e448915359b184fe7f12735b691a99df))

### Refactor

- regroup all 85 examples into OSI-layer folders ([`3345ad0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3345ad00f8d92dd816dc2537388991fee86ef45b))

</details>

## [3.8.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.1 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`da20458`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/da204580c19c4c26127f09d5a86462d7f8d9b4f9))
- update CHANGELOG.md [skip ci] ([`c73c4a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c73c4a33d2cd125a746a80ce7793ba9cf69b1977))

### Changes

- Bump version: 3.8.0 → 3.8.1 ([`c69d0d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c69d0d44c20df99c13ffd881d810fdc33331be16))

### Documentation

- surface OPC UA + full protocol set across the apex docs (audit) ([`daf1232`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/daf1232dcbba58200598135a64cddc4bac9e6667))

</details>

## [3.8.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.8.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`a3cb391`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3cb3912deeba2acd6b6fc03f7760944f15d2149))
- update CHANGELOG.md [skip ci] ([`1661f57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1661f57dc7d39448dfe9f73699ab6b04f9c20c2b))

### Changes

- Bump version: 3.7.1 → 3.8.0 ([`36b887b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36b887b6d02ecefd9d4d3ee345ba931d1810e751))

### Features

- OPC UA Write service (DataValue/Variant decode + write resolver) ([`72257a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72257a114071ecd1959d5c8bde9e7ac046697b34))

</details>

## [3.7.1] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.7.1 - 2026-06-28</b></summary>

### Bug Fixes

- spec-compliance bugs found by a library-wide protocol audit ([`f692b5c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f692b5cab83949ee84441ce4007972760f49045b))

### CI / Build

- update test report [skip ci] ([`51efec5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/51efec507479064d785c121b8559b9841eaec8e9))
- update CHANGELOG.md [skip ci] ([`511ea89`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/511ea894919797b15e48994fabca5dcc1ba13589))

### Changes

- Bump version: 3.7.0 → 3.7.1 ([`dc13764`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc137648b0d1690464ce9ef98196f676f02ec67a))

</details>

## [3.7.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.7.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`acef7bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/acef7bbc0e5d7b1b9b07431d1c2b5da4d622564c))
- update CHANGELOG.md [skip ci] ([`ed2f70a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed2f70a567f4d0f40bf8b95cbc7d8f05ddb13358))

### Changes

- Bump version: 3.6.0 → 3.7.0 ([`f57d2bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f57d2bc11e58108585379d68bf04a0f5f2c29a45))

### Features

- OPC UA GetEndpoints + ServiceFault + spec-compliant MSG framing (real-client interop) ([`ae0e493`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ae0e493854faf8cc1671f15f94c60f607c27742a))

</details>

## [3.6.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.6.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`376bcbe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/376bcbee03945f3de9cce98046252ad7967cbc09))
- update CHANGELOG.md [skip ci] ([`d88d7d6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d88d7d62154db614317a8b9b9a7ead0cab412ac3))

### Changes

- Bump version: 3.5.0 → 3.6.0 ([`f77ad11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f77ad11ec7de6d02ebf7d4f4e7edc942948d1588))

### Features

- OPC UA Binary client module (services/opcua_client) ([`6a70eae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a70eae9dab0f59918136b2c40b8f125a3b648fc))

</details>

## [3.5.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.5.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`5b7cca2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b7cca2b0b658daa7ca425aeab6723e297a37b05))
- update CHANGELOG.md [skip ci] ([`e697fd7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e697fd7251669f18e4e51b87996e4eef25e4116b))

### Changes

- Bump version: 3.4.0 → 3.5.0 ([`b3bf3b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b3bf3b9472675c6b5277a955b256d800f79b653c))

### Features

- OPC UA Browse service + CloseSession ([`e4e2a5a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e4e2a5a74be6277ba0f0b989d2b66b5a56f0ae10))

</details>

## [3.4.0] - 2026-06-28

<details>
<summary><b>Show Changelog for version 3.4.0 - 2026-06-28</b></summary>

### CI / Build

- update test report [skip ci] ([`06e4c6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06e4c6c1c5dde383bb498bbcc876b597f65b9494))
- update CHANGELOG.md [skip ci] ([`be198b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be198b4e39a611a3e7e7f4d3c2a8df4cbc7055c7))

### Changes

- Bump version: 3.3.0 → 3.4.0 ([`f9c6b6f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f9c6b6f5c85491e21749c549f869f258a69b853d))

### Features

- OPC UA Read service (Variant/DataValue + registered resolver) ([`09f46db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/09f46db7af655328ebea2c3685a83e622478fc44))

</details>

## [3.3.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.3.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3c61087`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c61087d09ef8c53800b2744c34c2603367817c2))
- update CHANGELOG.md [skip ci] ([`7f12c86`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7f12c86a552a851df2f29ee03414c88c7e3e23f5))

### Changes

- Bump version: 3.2.0 → 3.3.0 ([`4e26583`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e26583c4b6f13d496f79aaa8f27032da0abf28b))

### Features

- OPC UA Session (CreateSession + ActivateSession over MSG) ([`6b74f1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6b74f1c468adbd6cc4e0e5bd0bf6d4c2f949c519))

</details>

## [3.2.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.2.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`0b02e1a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b02e1a9501b8e1efa962ca038eea3ded1bffb15))
- update CHANGELOG.md [skip ci] ([`25895ce`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25895cefe92686af52642f07f5d1bac4b70999c3))

### Changes

- Bump version: 3.1.0 → 3.2.0 ([`30d0e0b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/30d0e0beae37148d2203c860c3e9f2a8be441490))

### Features

- OPC UA SecureChannel (OpenSecureChannel/OPN, SecurityPolicy None) ([`6297475`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6297475eabda802ed885dda5c5b6989d14c9e4bc))

</details>

## [3.1.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.1.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3fbe970`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3fbe97024296a995c0b4c12b7f472980a86f2b54))
- update CHANGELOG.md [skip ci] ([`c7ec6a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c7ec6a1dc05dc32104ff49d0bb26d1793a5db80e))

### Changes

- Bump version: 3.0.0 → 3.1.0 ([`9bc8b0f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9bc8b0ff0ffbe73b08b76095775ca532b7295091))

### Features

- OPC UA Binary server, increment 1 (UA-TCP + Hello/Acknowledge) ([`ead3edc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ead3edc29b3a54c0ff0a5e3960abd30d4a3ae9bb))

</details>

## [3.0.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 3.0.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`45680d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/45680d4d25e44de4e05a183791b4ef27f73fa5fe))
- update CHANGELOG.md [skip ci] ([`262d220`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/262d2203b36eddd5e0b3953c9bdbca821c0b0dff))

### Changes

- Bump version: 2.36.0 → 3.0.0 ([`f3aafd3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3aafd3e2807793a4f1d3d5ae1821a7102d40286))

</details>

## [2.36.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.36.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`873e4c7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/873e4c72c397105e9891f4829c0a7df46e717415))
- update CHANGELOG.md [skip ci] ([`f7fad94`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f7fad94f0389bb9e30731f4b90ded1187aa281ab))

### Changes

- Bump version: 2.35.0 → 2.36.0 ([`23d5dd5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23d5dd551f08d3cea96d39f76e46725f24f60994))

### Features

- OAuth2 token-endpoint client (authorization_code + refresh) ([`3a02051`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a020519525c3a6ea4ed7d33c467a715b00bcd1f))

</details>

## [2.35.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.35.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`4265730`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/42657303e7428f64b0d998d615a0c694eb964d62))
- update CHANGELOG.md [skip ci] ([`8d9a716`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8d9a7164ba1765438c904774da6580ac12851142))

### Changes

- Bump version: 2.34.0 → 2.35.0 ([`7769cb9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7769cb97b9118a1b3561842d0f5bac55e4d9e800))

### Features

- ESP-NOW peer messaging (typed envelope + peer registry) ([`0bf7b42`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0bf7b42707460845c7e26d586b9db8a1352636d6))

</details>

## [2.34.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.34.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`6096d83`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6096d832af3d59945adcb17a5808464888a62dfd))
- update CHANGELOG.md [skip ci] ([`8081b2c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8081b2cc520963501c777d6ef7deaf0943af65f6))

### Changes

- Bump version: 2.33.2 → 2.34.0 ([`4a6a379`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a6a379e988bf4f69e165b92ce7cd11097e9f860))

### Features

- GraphQL query subset (zero-heap parser + executor) ([`18ba5ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/18ba5ef6f8f1fe4d06a00344fdaca076be824afc))

### Refactor

- drop <stdlib.h> from the library (no-stdlib number parsing) ([`201f992`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/201f99286f2814c8ad970febb960682fe4e015dd))

</details>

## [2.33.2] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.33.2 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`fd2dde2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd2dde275cb89c7d58c4e1751e33b638d5ed4baa))
- update CHANGELOG.md [skip ci] ([`6dc1b34`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6dc1b34fc7733d6d3678e54e2dc46e374b9fa5ad))

### Changes

- Bump version: 2.33.1 → 2.33.2 ([`4112a9b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4112a9be0245f12e2db6f0e07ed2169269943aaa))

### Documentation

- ESP32 Secure Boot + Flash Encryption hardening guide ([`ebf288c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ebf288c16b994dccc8d701cb75f3cc51cd3b239d))

</details>

## [2.33.1] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.33.1 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`0eaf4d2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0eaf4d22e0e2d04744688f0404acdbabf270d20d))
- update CHANGELOG.md [skip ci] ([`25854e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25854e04e8afff16d6728b0a28ec046595280349))

### Changes

- Bump version: 2.33.0 → 2.33.1 ([`862d620`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/862d620fb16990b84f31872f94b30a19f9ef41d5))

### Documentation

- feature table with hover tooltips + docs/FEATURES.md ([`854ccb2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/854ccb256307cf39ea4e7130b09b3d1a758a35dc))

</details>

## [2.33.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.33.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3b540d4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b540d46b3674c64f15de342015a0e5650eda2e1))
- update CHANGELOG.md [skip ci] ([`f8c5fc9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f8c5fc992dabecdb57e7cc695d93300d144acfff))

### Changes

- Bump version: 2.32.0 → 2.33.0 ([`4b7fb09`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4b7fb096804904d5a9d1c379bcea48c2f934c5a3))

### Features

- unified VFS wrapper (RAM + Arduino FS backends) ([`7446668`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7446668c001ff9a734d5fd164c69c44569f03999))

</details>

## [2.32.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.32.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`9ee1ca3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ee1ca327626d7189650a148cb59c589947537a0))
- update CHANGELOG.md [skip ci] ([`554307d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/554307d182c7cb4e2fb4e1f54413628c30ab3994))

### Changes

- Bump version: 2.31.0 → 2.32.0 ([`36dbb84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/36dbb841450e08e1c7c644088de31885ec72e584))

### Features

- OpenID Connect ID-token verification (RS256) ([`5c21aaf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c21aafb1c715e6fa497b1cecef5115e198ec759))

</details>

## [2.31.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.31.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`bf5d9f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf5d9f5d65af32416577ecbc6ece2bbe5645bfa3))
- update CHANGELOG.md [skip ci] ([`84c16d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84c16d1e19e2d979a57aa97e6b087e09b6d30cf8))

### Changes

- Bump version: 2.30.0 → 2.31.0 ([`3a33f18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a33f187205ecc363115dc945dfc626637311840))

### Features

- tamper-evident hash-chained audit log ([`012b6e3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/012b6e3ac2d44fdae160e6ff53298e1644749d10))

</details>

## [2.30.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.30.0 - 2026-06-27</b></summary>

### Changes

- Bump version: 2.29.1 → 2.30.0 ([`711bf43`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/711bf43d5aa8f520659216c66600908a178e8a4e))

### Features

- WebSocket permessage-deflate outbound compression ([`ef9babd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef9babdc1081609090acfe685970fddab28f0d0a))

</details>

## [2.29.1] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.29.1 - 2026-06-27</b></summary>

### Bug Fixes

- never HTTP-parse a WebSocket-upgraded slot (first-connection drop) ([`dc83dd5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc83dd5b406dbc30ae01b517fac6cbca988c5a8e))

### CI / Build

- update test report [skip ci] ([`4ac1ce6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4ac1ce6ba09f8746793d5c1f2698b5b592869ff8))
- update CHANGELOG.md [skip ci] ([`7db045d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7db045d30dbbc5e3df2f96a447b5dbcd1a5acd6e))

### Changes

- Bump version: 2.29.0 → 2.29.1 ([`976d66a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/976d66abca34e8d1b5db13e629a080c849f42c06))

</details>

## [2.29.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.29.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`f39cb16`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f39cb169d71806247924517360e3260ffaa98075))
- update CHANGELOG.md [skip ci] ([`c136f6c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c136f6cf6acdcd3200e0451d176cd4f855f53218))

### Changes

- Bump version: 2.28.0 → 2.29.0 ([`7092cb2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7092cb29eafcd6178cb223b2e15106bb1d91d482))

### Features

- DNS resolver with answer verification ([`f904e4a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f904e4ab220175c59d2fa8fb8ecf8746fc2d41d7))

</details>

## [2.28.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.28.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`786ab30`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/786ab3077a4922ace8995aea6a8d79744a038d86))
- update CHANGELOG.md [skip ci] ([`0ca3b74`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ca3b74d411b98784e9509c037f3e1aa94e6e630))
- update test report [skip ci] ([`15f3582`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/15f3582bf5fe65394171db6310d5f6fac51c6b85))
- update CHANGELOG.md [skip ci] ([`94c7b88`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/94c7b887cf074e43caab76e663d63a4862c027e5))

### Changes

- Bump version: 2.27.0 → 2.28.0 ([`d311adf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d311adfafe53d95c3d2114ae1cc1b1a20becc82d))

### Features

- WiFi radio power controls (modem-sleep + TX cap) ([`60333ff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/60333ff3779d2fcc03a9b22ad56650cdfe45d84d))
- pentesting / adversarial suite + guide ([`68e1c6a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/68e1c6ab3b832e687785ab75c0e1c7f32b7c4a97))

</details>

## [2.27.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.27.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`1f6ea1e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f6ea1efd27b1f957f535e98b5f20783ed5cd9e8))
- update CHANGELOG.md [skip ci] ([`731bf19`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/731bf198007a2115a1cd6b6b1639a1c5e8f5f06e))

### Changes

- Bump version: 2.26.0 → 2.27.0 ([`0649646`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0649646b4ac5570bb8a5f01354150cff2c239c32))

### Features

- outbound webhooks / IFTTT ([`655951b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/655951b241a90f3b8c56c9cabe0f52c96144a025))

</details>

## [2.26.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.26.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`519d982`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/519d982c6a889d97d0eec1cb06a7c00be550011f))
- update CHANGELOG.md [skip ci] ([`59ab16a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/59ab16aabb2322b08bd7995add31089e3aacac4c))

### Changes

- Bump version: 2.25.0 → 2.26.0 ([`6f49e2b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6f49e2bedb4068f2d1c5ff42fbef7556c9212dba))

### Features

- TOTP two-factor auth (RFC 6238) ([`a718d5b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a718d5b6c1a1483011b53f8882b5eb805ba9e7e9))

</details>

## [2.25.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.25.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`25b1402`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/25b14028f9d5fc5d29100674b6846f779bea435b))
- update CHANGELOG.md [skip ci] ([`6793b1a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6793b1afbb3b1b5252c1b3c9f4fb63a7292b5e2d))

### Changes

- Bump version: 2.24.0 → 2.25.0 ([`6ecbff6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ecbff631cd607465ce8f78d117992e7573da61c))

### Features

- pluggable monotonic clock + compile-time worker poll-rate knob ([`c634a5e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c634a5e14ec2ca6a37faafae519743930e8704c3))

</details>

## [2.24.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.24.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`ba1e38f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba1e38fe4a874d300447083febb6be7d5e45314a))
- update CHANGELOG.md [skip ci] ([`92d8b02`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92d8b02eef5e0c059c93d61e6b2dafa79f1ff37f))

### Changes

- Bump version: 2.23.0 → 2.24.0 ([`9d38b0d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9d38b0df5e3b50e53194c9cdb65b70227c64f0a3))

### Features

- OTA rollback protection + soft-brick safeguard ([`5a022a7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a022a784661d44d06a9100990d0c2d79d6ffd6c))

</details>

## [2.23.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.23.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`b53b085`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b53b085477dbb33f5b32d3b168ad7678fd5aa61c))
- update CHANGELOG.md [skip ci] ([`328ae49`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/328ae49d04623412a7f1c0ee39f80bd8671a53bb))

### Changes

- Bump version: 2.22.0 → 2.23.0 ([`eab8e04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eab8e04dec3630c7708cb15c1fc5ed55cbca8382))

### Features

- Modbus master codec + register scanner ([`efa789d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/efa789db5f7a9df90ca865477c35e917f7c08154))

</details>

## [2.22.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.22.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`3b428a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b428a066f9017107448e733a03f6d1953e31df5))
- update CHANGELOG.md [skip ci] ([`6c7fffe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6c7fffed89d5368b433d0fccea79c14cf2f07195))

### Changes

- Bump version: 2.21.0 → 2.22.0 ([`26b7f48`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/26b7f48b68aa61b13cf878952ddf21791211e686))

### Features

- schema-driven config export/restore ([`08ee802`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08ee8028aade673ac4e536078fc5321ee7561f32))

</details>

## [2.21.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.21.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`c98657a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c98657a5fbf671acb8e06e2b1e478a9b4b421e14))
- update CHANGELOG.md [skip ci] ([`90a34f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/90a34f769f68dedc7430f084510ad6d0a7ff88e3))

### Changes

- Bump version: 2.20.0 → 2.21.0 ([`ce1bcca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ce1bccade8d3241b2375e7ea738e1098cd06ffa9))

### Features

- fixed-RAM rotating log buffer with severity traps ([`23af94a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23af94a14f5b4c4e557b39e55e2dca3a2fb9dcc5))

</details>

## [2.20.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.20.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`1b5cd23`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1b5cd23ca975e2d73c0b53fe21106f5fd4e95eb1))
- update CHANGELOG.md [skip ci] ([`e7b971d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e7b971dcd1cdf89401fcd259c06e1769a177b1f4))

### Changes

- Bump version: 2.19.0 → 2.20.0 ([`794a655`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/794a65523dbcee80bb8f0ec8f8319aa2b137ce6d))

### Features

- runtime heap/stack guardrails with breach callback ([`fae0b29`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fae0b297b8d5d2cc462fcacce39d491f4249e5dc))

</details>

## [2.19.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.19.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`32f2772`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/32f27721ebea318705b98bd2494b9dc005980b4d))
- update CHANGELOG.md [skip ci] ([`f31a1c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f31a1c6537d400484c35be98438a2b0e0b633c55))
- update test report [skip ci] ([`a350a0f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a350a0f647bbfd97e0bc9d590e932d07dd92d6cc))
- update CHANGELOG.md [skip ci] ([`c4a6da3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4a6da3d7777e9be2395e56f93240f3db9f2134d))
- update test report [skip ci] ([`a244f15`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a244f1533fa70523a98a508fd7d1aed3ccc90da3))
- update CHANGELOG.md [skip ci] ([`338b51f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/338b51f79fe9b47746b4efcaf4b99ba39e4bd703))

### Changes

- Bump version: 2.18.0 → 2.19.0 ([`a5541c8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a5541c8039ac0da840d6df0c1835a958317f9b48))

### Documentation

- record worker throughput benchmark (N=2 ~1.5x under concurrency) ([`e7fb89b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e7fb89b28f8e491b48fb8dd16e7c74b602baa289))
- document the worker model (concurrency roadmap + README) ([`8cf7838`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8cf78389d505a6ace7462ffad5b7341cb3ee7759))

### Features

- zero-heap UDP telemetry cast (InfluxDB line protocol) ([`2c5e156`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c5e15691c04d8d943142aac3b73b36ee42d16e4))

</details>

## [2.18.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.18.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`563f71e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/563f71ec4ff2a1d78592009a681286ce4f9ac6a5))
- update CHANGELOG.md [skip ci] ([`6a18be1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6a18be19c3953295c276908b8e304af5c83262f2))

### Changes

- Bump version: 2.17.0 → 2.18.0 ([`a3b72f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a3b72f25dc188c5eff27b36981c3a154538c5be9))

### Features

- thread-safe deferred-callback path for app pushes to workers ([`ac0eccd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac0eccd0942c2c67d1b1757e7fedf82abbb883d5))

</details>

## [2.17.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.17.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`6d4bf5b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d4bf5b787a7349da1b9ae6ed0d9bab59fbcc8fc))
- update CHANGELOG.md [skip ci] ([`e6254de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e6254deab6a13ff1a2468da802adb9a723aa4db1))

### Changes

- Bump version: 2.16.0 → 2.17.0 ([`024739f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/024739fb0241fcf25ffc0ae49751618dea8c63c4))

### Features

- core-partitioned parallel workers (DETWS_WORKER_COUNT > 1) ([`60baa1b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/60baa1b3e9858bc3f71775aac687a0f63b132576))

</details>

## [2.16.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.16.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`5196ba6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5196ba68f9220071c04efb4438e86b6b2df38b6e))
- update CHANGELOG.md [skip ci] ([`3f9c68c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f9c68cfa795448220d637b29da3a96bd31731d7))

### Changes

- Bump version: 2.15.0 → 2.16.0 ([`9ab38f5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9ab38f5d4dd2f18d4fb822cef3c118be5febd145))

### Features

- run the server in a dedicated worker task, freeing loop() ([`0768a4f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0768a4f741e375472515533835296e4a9a0c5ed6))

</details>

## [2.15.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.15.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`9eac7a2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9eac7a25badf0185301545cf009f1b086740d88e))
- update CHANGELOG.md [skip ci] ([`23ace18`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/23ace18a7cbb48407c3c3da7eca0210b91d939c3))
- add msgpack encoding terms and changelog hash to cspell dictionary ([`6f7b56b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6f7b56b7617bd8340aa781e703bd629b42c94ee8))
- update test report [skip ci] ([`7b62407`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7b624075e9903865eb469e711620de9fbf1ff99a))
- update CHANGELOG.md [skip ci] ([`a8c7b19`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8c7b19104d3492447b44a8bdc0f2f7f9d494012))

### Changes

- Bump version: 2.14.0 → 2.15.0 ([`fa3b83b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa3b83b4448620665c917c19f4287a87c180541e))

### Features

- thread-safe transport groundwork for the worker model ([`71dbd72`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/71dbd729d978b94280865f6101c452540af395ac))

</details>

## [2.14.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.14.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`554907b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/554907baf6e2b4e6bb7b523fb7e50b6b86221b37))
- update CHANGELOG.md [skip ci] ([`de76522`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/de765223f70d451beb832491e0e0acc81f16c5ed))

### Changes

- Bump version: 2.13.0 → 2.14.0 ([`48e679b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/48e679b1b4a9dbf82c2f797a5d2b627497c81c76))

### Features

- browser GPIO pin-mapper diagnostics endpoint ([`c0857bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c0857bc1e96ed89d447e4c62e68424ed44a3ea25))

</details>

## [2.13.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.13.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`f0bc212`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f0bc212a07ba92406158ef2238c371e7ba0b6dbc))
- update CHANGELOG.md [skip ci] ([`8b0cffe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8b0cffec4fd1b6a143c428215f8ffc76876dbd83))

### Changes

- Bump version: 2.12.0 → 2.13.0 ([`ed77db5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ed77db54b56786b74db5bec76a99d28f814d7894))

### Features

- zero-heap MessagePack encoder ([`0354ffe`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0354ffe1d8d3d79c59baef7ccf8c6b8662f00980))

</details>

## [2.12.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.12.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`12ed60f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12ed60f58d13198203edc13de39fa798c4e5117a))
- update CHANGELOG.md [skip ci] ([`110e56c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/110e56c106143067e19db4a90964e7ea2d08b681))

### Changes

- Bump version: 2.11.0 → 2.12.0 ([`fc9cacb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fc9cacbdd20de5191165dcf6f32e66c0d253eae8))

### Features

- granular JWT scope/role authorization helpers ([`5dfedb1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5dfedb1573bb98eaf875e48fceb6b976a53eb041))

</details>

## [2.11.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.11.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`a20f887`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a20f8878dc8149612e0534f4475f7a9d5f2dc729))
- update CHANGELOG.md [skip ci] ([`b9da393`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b9da3937bac349927eb4d3f9f7562bcd33a85d1d))

### Changes

- Bump version: 2.10.0 → 2.11.0 ([`1ae8c46`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1ae8c4674fc958cc5caaf63bc9ec726b481bf109))

### Features

- CBOR decoder (cursor reader) ([`c053c07`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c053c07c3b8416af29806f4119e0c70f804c14f4))

</details>

## [2.10.0] - 2026-06-27

<details>
<summary><b>Show Changelog for version 2.10.0 - 2026-06-27</b></summary>

### CI / Build

- update test report [skip ci] ([`847014c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/847014c74a08e87dfe6c107276190abf4ff7423b))
- update CHANGELOG.md [skip ci] ([`305ded5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/305ded54d8b08b5921e97e9e55c743a70cf79ac2))

### Changes

- Bump version: 2.9.0 → 2.10.0 ([`d1756eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d1756eb940d611b5a621f30377b2320f6fa7025a))

### Features

- zero-heap CBOR (RFC 8949) encoder ([`00a4bae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00a4bae29976a28368de6ed1ee4f005fca8c4264))

</details>

## [2.9.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.9.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`2b1c080`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2b1c08009e7a38a77b38b7d2cd7ea6fa6d3df968))
- update CHANGELOG.md [skip ci] ([`dd4ba62`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dd4ba627d2264418c6e97f2ec7b7ba8963af6173))

### Changes

- Bump version: 2.8.0 → 2.9.0 ([`67243c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/67243c0bba3ad6dfa15bcb14ec09a37b41bb9364))

### Features

- flash partition-map monitor endpoint ([`5fbc48a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5fbc48af0a1f93a8827694db4ba2ca13a01cceee))

</details>

## [2.8.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.8.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`c264beb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c264bebdb5d1d1b410e1febd710681feff77ac06))
- update CHANGELOG.md [skip ci] ([`aedff6a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aedff6a05fc9af1d6e44f75377bace8d33bbe0c0))

### Changes

- Bump version: 2.7.0 → 2.8.0 ([`2e20222`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2e202224dd1e2ae0a62f743dc278a4645c5d29b1))

### Features

- egress-interface reporting (det_net_egress) ([`56682af`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/56682affa12a8cd16e91b3c7c7902ff3f01a65ab))

</details>

## [2.7.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.7.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`a9ced97`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a9ced9750067560c88cdaf32b60f0b4170b1e215))
- update CHANGELOG.md [skip ci] ([`7050872`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/70508720daa67712f70a5daa7eb1a60abff3952c))

### Changes

- Bump version: 2.6.0 → 2.7.0 ([`f32cf87`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f32cf876ea97726950cabccb6c311788350a5f15))

### Features

- dashboard WebSocket controls and Canvas chart (phase 2) ([`dc7e703`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dc7e7038c3b0bed260168271c8138e5bf18628d7))

</details>

## [2.6.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.6.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`4929167`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4929167fa8f37158a860b681b05b8e5c9da3a338))
- update CHANGELOG.md [skip ci] ([`04f3061`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04f3061272c6f4bfb9b337032d249087cbad1f63))

### Changes

- Bump version: 2.5.0 → 2.6.0 ([`bfa2483`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bfa2483ed078cffb543e262928143d2df891b706))
- clang-format the 61.Telemetry example ([`a59a546`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a59a54674e6674e034c8681069c0528b9ae95cb7))

### Features

- real-time SVG dashboard over SSE (phase 1) ([`048c457`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/048c457a0bb218c78e1d10afd260260414329d0d))

</details>

## [2.5.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.5.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`91d304b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/91d304b0c01c809501119a52dc29b0558f1daa40))
- update CHANGELOG.md [skip ci] ([`e8de582`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e8de58237028642cb88b915460c350019200c9cc))
- update test report [skip ci] ([`ace19b3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ace19b392c81ad1454e37005f16de85aca89c275))
- update CHANGELOG.md [skip ci] ([`4558d97`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4558d97375e89e617a64a01abd8fb191cced7976))

### Changes

- Bump version: 2.4.1 → 2.5.0 ([`9922d9d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9922d9de9e177fc24e491faef268c9848dedc99e))

### Documentation

- link CodeQL / Roadmap / Known-Limitations pages from the docs index ([`334028a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/334028ac02ae427e92acf7b350192bbb538bc80f))

### Features

- telemetry math helpers (moving-window stats, rate-of-change, totalizer) ([`84c6170`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84c6170b1aed6f91e8c2038acbee2b57f456e516))

</details>

## [2.4.1] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.4.1 - 2026-06-26</b></summary>

### CI / Build

- registry-ready library.json (homepage, headers, export rules) ([`2a61268`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2a61268293161a17735cbe64f578e982bfd03966))
- update test report [skip ci] ([`fcca10b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fcca10b61212b0652070fdd03e895586ca8add06))
- update CHANGELOG.md [skip ci] ([`fd1a2b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd1a2b269ea2acf52c4abb8fef7c208de57f7dc4))
- expand CodeQL coverage to the new modules and integration paths ([`3703e86`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3703e86676f41d74d2d243e46889b28a5a383700))
- update test report [skip ci] ([`f9d355a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f9d355a264bdd75eb69a6cc65a53aaafeb0a2919))
- update CHANGELOG.md [skip ci] ([`19d3a76`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/19d3a76d28ba6b5d544ede79f1a3a8f8c6989e1f))
- update test report [skip ci] ([`f935fad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f935fad0552c2a9bddb20d3725947565450e0b11))
- update CHANGELOG.md [skip ci] ([`fd4e3a0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd4e3a031a4a13ee2d7836d8083eddfd3c18db4e))
- only SSH-sign bot commits when the signing key secret is present ([`a7543fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a7543fac993055958561e5e2321b4350cc00d89e))

### Changes

- Bump version: 2.4.0 → 2.4.1 ([`7db5bf5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7db5bf591bb4ffb3892bdee86fa4b6d1cce1d45a))
- less homicidal squirty ([`edbb9d6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/edbb9d66ba831bca3bdb49a62220ec9a684d1f6f))

### Documentation

- add pentesting / adversarial suite to roadmap ([`b386a50`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b386a50473c44d12f08dfa3951f40fd94bb42cbb))
- group README features by OSI layer in collapsible blocks ([`01d95f7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01d95f7335d861871075dc633ce94ead1a896028))

</details>

## [2.4.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.4.0 - 2026-06-26</b></summary>

### CI / Build

- SSH-sign the changelog and test-report bot commits ([`dfa89e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dfa89e4e23479b9f05cf7a2d89074b15d3ab773c))
- push changelog and test-report via the automatic-actions deploy key ([`940dece`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/940dece0740510c78d5f6ca136c4dcb31354aa13))

### Changes

- Bump version: 2.3.0 → 2.4.0 ([`4185c7f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4185c7feed2959dc271db6c26907a673d7c45e24))

### Features

- CSRF protection for state-changing requests ([`6d0e17c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d0e17c19d6fd68f7a6efe3428b0df16ff05c9fd))

</details>

## [2.3.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.3.0 - 2026-06-26</b></summary>

### Changes

- Bump version: 2.2.0 → 2.3.0 ([`a6cdac1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a6cdac12a6c1a35a8ad0f0c4fa0ed8eed2d9486d))

### Features

- per-IP brute-force lockout for HTTP auth ([`1bcdd03`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1bcdd03d0ee700482ee3bb54bc2cbb535b773d70))

</details>

## [2.2.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.2.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`0c199a3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0c199a33eb912ada1dcad26dc671745911d69376))
- update CHANGELOG.md [skip ci] ([`3fd5992`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3fd59927ac647abeda8b1caa3dd19cd1ebf30d6a))
- update test report [skip ci] ([`aae0666`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aae066607f952e0a54f87749a3365832090be093))
- update CHANGELOG.md [skip ci] ([`d21d17d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d21d17d01b311e95894c407229c36b4dca6e8e74))
- add CodeQL analysis workflow + badge ([`e3ef7df`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e3ef7dfa2bfe6f3e804997d086432ca5fb69b00d))
- update test report [skip ci] ([`04b7ae8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04b7ae8c46a818a22549ffb35dfb2c66aa35cc54))
- update CHANGELOG.md [skip ci] ([`3c82c25`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3c82c25a5e02a1fcdb192223fdcd9c5c438350e0))
- bump actions/checkout from 4 to 7 ([`1154ff1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1154ff1422fd14f1a05d96248a9d4331ffd4e629))
- update test report [skip ci] ([`64845f9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64845f977118e7e16040070efebc0f96b327add5))
- update CHANGELOG.md [skip ci] ([`f96406e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f96406e0fae8e405dd346e296cabfe9e4b7e727f))
- update test report [skip ci] ([`9f7c6f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f7c6f2ded07403a92539892697c34039933860e))
- update CHANGELOG.md [skip ci] ([`f2b4a11`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2b4a1192999eedaf240bd59e462ed711e080f33))
- update test report [skip ci] ([`07b3802`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/07b380240a8a6b25b94a010045ba4ed096251e4e))
- update CHANGELOG.md [skip ci] ([`571ffc5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/571ffc51893dca1e41aa200231d994f9b58c755b))
- update CHANGELOG.md [skip ci] ([`b633663`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b63366344e35f19159119a1263dafadbd2422c83))

### Changes

- Bump version: 2.1.2 → 2.2.0 ([`e525ad2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e525ad2bd2bcbc3f7bb64adc221ef19daaa1ee4d))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`0ce576e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0ce576edba78205116954d5bd764fe497d685296))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`e399d4a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e399d4a4eafc1e8c59abf10f5ffa7860fb5481f1))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`522ae47`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/522ae4741923a34120a88ba0784e35d83cf1d818))
- Merge pull request #3 from dstroy0/dependabot/github_actions/actions/checkout-7 ([`e12efa8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e12efa8c9a45e5278910140fdd6c106aca27f675))
- codeql workflow ([`fc43f63`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fc43f63f38541d22ea2a5081221609614226a80c))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ff7a9e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ff7a9e9def10229bab270420d81fd744fdcecd29))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`059af85`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/059af85e7289728f892d8eafdf5b19d26bfc3de9))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`debec1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/debec1dbbd49b4fce7d0344bae167a9490d52731))

### Documentation

- fill in CodeQL findings (no security issues) ([`5ab3515`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5ab35156a888782c07ff5c880d3b83bef9cceebd))

### Features

- source-IP allowlist (accept-time firewall) ([`077e3d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/077e3d061e78103dcf8f315c39fb1f8d52e280e2))
- mDNS/Bonjour TXT records and extra service advertisement ([`4c1a23d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c1a23d37f12faa60c8494dcc8480111d7e5b623))
- MAC-derived device UUID (RFC 4122 v5) ([`c9020f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c9020f4d89c804f9e5645cce13fbeaf4ab373eaf))
- Cache-Control header for static files ([`d2247dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d2247dd8d050bf1afc920579b0a34f80efec31ee))

### Refactor

- SSH scratch tenants + ROADMAP/KNOWN_LIMITATIONS split ([`6d7eedb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d7eedb21214040c7a6d4f6b080af8f28a669f0c))

</details>

## [2.1.2] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.1.2 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`3b594ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3b594edd17eae90ed9fdcfd773708925669d18e5))
- update CHANGELOG.md [skip ci] ([`e96c884`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e96c8845eda7e6b3e893ed98853c5a2014f17fdf))
- update CHANGELOG.md [skip ci] ([`d35218a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d35218a704b28185f92bcd27ed129826a814986c))

### Changes

- Bump version: 2.1.1 → 2.1.2 ([`8e52f4b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8e52f4b9c40c0f00291b233003248dc9675ee09e))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f831485`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f831485243c8c2567015ef2244ece510f7504f59))

### Features

- scratch pool, permessage-deflate, time-source + config-store services ([`e01c07c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e01c07c31da72cd5d5b8d7bcda22ad4e42b630bb))

</details>

## [2.1.1] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.1.1 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`017478b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/017478b88121e1cb80427220254abb8c856c7af5))
- update CHANGELOG.md [skip ci] ([`c558f86`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c558f86ab7f71957a1eeafe245b543a2efb661d6))
- update CHANGELOG.md [skip ci] ([`c4768db`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4768db7145a17e156491d89c067a206abfd2c39))

### Changes

- Bump version: 2.1.0 → 2.1.1 ([`0a82616`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a82616c46834fde553f52008d6882ca8eaa9a26))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`cf77381`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cf77381344ac6c034ce518599cc113160542889b))

### Refactor

- consolidate generated assets into one unit; template stats() ([`1727f80`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1727f8045ec7a998c3b5f9efe190f101e6485acf))

</details>

## [2.1.0] - 2026-06-26

<details>
<summary><b>Show Changelog for version 2.1.0 - 2026-06-26</b></summary>

### CI / Build

- update test report [skip ci] ([`a06e0ac`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a06e0acbad7160dcf99ba20950899cc39e04c128))
- update CHANGELOG.md [skip ci] ([`64a8328`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64a8328aa86ae5e2ab4ca45c21f2a7f79cedac07))
- update test report [skip ci] ([`0a81789`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0a81789452ac4234eba8e3ca84670e692964e544))
- update CHANGELOG.md [skip ci] ([`7ab520b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ab520b74fa3188aed9b9d9380acabdf85c35038))
- update test report [skip ci] ([`caab45b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/caab45b421a377b6ce0084320367f7de625a14dc))
- update CHANGELOG.md [skip ci] ([`5e1c641`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5e1c641641678864cabd632f0224b86aff57e1a1))
- update test report [skip ci] ([`60b3a0a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/60b3a0ad29ac2b322b4a9497aae7315f57edabe6))
- update CHANGELOG.md [skip ci] ([`fbadd3d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fbadd3dd46aacda1862b3246893269e0913caa81))
- update test report [skip ci] ([`d81d0e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d81d0e19bfa305de9b1c567a937e46d579ea55c0))
- update CHANGELOG.md [skip ci] ([`da969e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/da969e0d26f73bd1c403b14bcd30dedb79fdc169))
- update test report [skip ci] ([`8dabf46`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8dabf46c081ab4bbe846f59620ed3f2fbb99de43))
- update CHANGELOG.md [skip ci] ([`d99d2b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d99d2b97c5ae33e69268eca58dbdc35ac60c277e))
- update test report [skip ci] ([`64b7d3a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/64b7d3afc0147a3fb4248a600f26cd2846909582))
- update CHANGELOG.md [skip ci] ([`96631a1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96631a1c3a9fc22de79d22b6eea0b6054d2ff085))
- update test report [skip ci] ([`988e980`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/988e980173dbc204a3fa34af14c425b5426af315))
- update CHANGELOG.md [skip ci] ([`266d4dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/266d4ddc5bdcb9d25954c2f7cc27cde6a1d03f59))
- update test report [skip ci] ([`96b22e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/96b22e1d07ae1a201e1add7faf668eed4782f0bf))
- update CHANGELOG.md [skip ci] ([`f2478b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f2478b2d0413564fe57b0e3c3840614e6a668a4b))
- update test report [skip ci] ([`01f8a40`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01f8a40a121f1be22dbc2cf32011b8ada69f3234))
- update CHANGELOG.md [skip ci] ([`45b7a55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/45b7a55801ff3afa1798f0bdaf742bf322840087))
- update test report [skip ci] ([`2342b47`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2342b475bbf0e35fdcbd35eda6ab5d275583f11e))
- update CHANGELOG.md [skip ci] ([`401716a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/401716a55530d86c4738aebcd404526ce6180401))
- update test report [skip ci] ([`33610dd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/33610dd20f2adecf4c4c93bdd1bd368e4adf92aa))
- update CHANGELOG.md [skip ci] ([`46480dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46480dcd01f64dcdea79e7d56e2edeeffe6f7ebc))
- update test report [skip ci] ([`3eff235`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3eff235f193ca20559adf87d12feff2196c3830e))
- update CHANGELOG.md [skip ci] ([`de2a3f1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/de2a3f1f9e37d72bcb03b31e0f77cdafea186b5e))
- update test report [skip ci] ([`50583ed`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/50583ed86388381a88c0e6af562c56806a0967ab))
- update CHANGELOG.md [skip ci] ([`7d7ea83`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7d7ea83adfa2d0d7cf437530cd9a842b1d5dcb06))

### Changes

- Bump version: 2.0.0 → 2.1.0 ([`0f8289c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0f8289c0d0a8d9c00c2911308b1bb4b62a86eec9))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`716f59c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/716f59cdfe86932ab6bc9406096493d90241205d))
- Merge branches 'main' and 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`cacd0a8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cacd0a8dec596e3282b989309e38ed6f4099e0ed))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`bf82439`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bf8243923350fced29314724fdbf211346ea18d4))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`4e7e870`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e7e87091d4fb52d7d64c79f879bbfc6c0d61e15))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ca360f8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ca360f8354fee579f1b26cf88286b58768d07056))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`d7bcc90`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d7bcc900c43237a2c4b3eadb12cbeb6c416d0455))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`1b691fd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1b691fdf0248ccbfa23a3f153e61a770bb5682ce))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`770c011`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/770c011376e31bbb222ac3c076a6b18cea062428))
- Merge branches 'main' and 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`7290bff`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7290bff7052ecd6b39a16eda6eae1b4a552aa997))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`9fb00f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9fb00f6ca195e3d02c7feb97b37ee966755398a1))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ba88aad`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ba88aad8758d3e6519d7230b5f427f82e13bd9a7))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c60f4b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c60f4b9d43ead45ee5c9b73e304bf1322e4ef36f))

### Documentation

- format ([`a8c2c55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a8c2c55a1a669024f5f0d9589f40ea21f68bd1fd))
- smile, 9-way eyes, swim rotation, live-rock house, sticky timeout ([`4f7c5e9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4f7c5e991190edd7ccb9aa415eba58b4b0a3b858))

### Features

- TLS session resumption - RFC 5077 session tickets ([`fd0c350`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fd0c35008ad71e5f583a265ea4a8c7e7576f2ec5))
- Modbus TCP slave - Modbus Application Protocol on TCP/502 ([`6d732c0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6d732c006aec173b64a531837da291cacfb46982))
- WebDAV server - RFC 4918 (class 1 + advisory locks) ([`5b09b48`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b09b485f7061aaa13f3862907a04afe61105dd5))
- per-IP accept-rate throttle (connection-flood defense) ([`d5364d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d5364d0afd2731efeadd306688d16193e9b732d1))
- CoAP block-wise transfer - RFC 7959 (Block1/Block2) ([`191090f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/191090f56b3e7aa62b0cbbb9c8c27a4fb5ae733b))
- CoAP resource observation - RFC 7641 (Observe) ([`d8ed477`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d8ed47796f94352e596d8c2336e9316b7814c58c))
- outbound SNMP notifications - traps and informs (v2c + v3) ([`98ce997`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/98ce9979f42a012258e2ee696cf558fd98dac731))
- outbound WebSocket client + wss (RFC 6455) ([`a0d0b57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d0b57b4ac8b4da55140fc85c22e43faec23426))
- MQTT 3.1.1 client + MQTTS, full QoS 0/1/2 ([`4c8191d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c8191dd92ddb52e59883c86b89e8c28c3f5901f))
- pluggable protocol dispatch, flow control, and outbound TLS auth ([`0417d04`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0417d04af1ab7d134f3086805576e1b24cd5d4a1))
- add 10 optional subsystems, harden transport, regroup examples ([`9028bfa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9028bfa1481df7f69cd865660091e1807fab67dc))
- add 10 optional subsystems, harden transport, regroup examples ([`cfa9ac4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cfa9ac4dfcc1f2590c2ec7153bd456781b99ed48))

### Refactor

- externalize served strings to src/web with a deterministic asset generator ([`7be3e55`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7be3e550fadc54a642d8dad2bc8ef96898e92c03))

</details>

## [2.0.0] - 2026-06-24

<details>
<summary><b>Show Changelog for version 2.0.0 - 2026-06-24</b></summary>

### CI / Build

- update test report [skip ci] ([`b11f1f3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b11f1f3a1ba0a45acdb0843427ca23e46dcfffa1))
- update CHANGELOG.md [skip ci] ([`e60eb61`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e60eb612a7aa3dbed15785a99d29cca2609a47b2))
- update test report [skip ci] ([`836608c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/836608c5565f4f429ac84f3b859879b032da264b))
- update CHANGELOG.md [skip ci] ([`04d2147`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/04d21470f09e199708fd94ea8c15822d04530721))
- update CHANGELOG.md [skip ci] ([`1a0f678`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a0f6782232ff2807383e36f2f25ef23e0566812))

### Changes

- Bump version: 1.2.7 → 2.0.0 ([`ab9f558`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ab9f55868206d3fa90e2acfe21f90d1dc68fe16b))
- formatting ([`9a67e81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9a67e81df024b4d365deaf6ae5d7079f2c0a7400))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`be35eb2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/be35eb2bd4579752bb7bd346894482ae153e1883))

### Refactor

- transport-layer I/O API + Telnet server; reorganize examples; docs ([`d34d51a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d34d51adb754b206039e68e3d6b286d88edc6e10))

</details>

## [1.2.7] - 2026-06-24

<details>
<summary><b>Show Changelog for version 1.2.7 - 2026-06-24</b></summary>

### CI / Build

- update test report [skip ci] ([`42b0fd7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/42b0fd7331cff5a81abc589b191e895549e10e92))
- update CHANGELOG.md [skip ci] ([`c718cf3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c718cf32b1d18b41f42abf7741d676e92a41e83c))
- update test report [skip ci] ([`2c56ddc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2c56ddcd0eb59a0d5af3078a153d36dd9e05ad65))
- update CHANGELOG.md [skip ci] ([`e40c020`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e40c0203edfdb7be4c4ec6f7a88f188aa0453d90))
- bump streetsidesoftware/cspell-action from 6 to 8 ([`9b3e289`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b3e2898c3e76359af1444dc2e2fb001ce26f7d5))
- update test report [skip ci] ([`835e317`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/835e3175a9f64b760581a74a1fe63e462732c84f))
- update CHANGELOG.md [skip ci] ([`46368f3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46368f37030ba94efc90e308cd4a9062b77ee6c4))
- update CHANGELOG.md [skip ci] ([`1d7c16c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1d7c16c4407dc75c1f00fc45cfcd12590bd3df59))

### Changes

- Bump version: 1.2.6 → 1.2.7 ([`782a0bb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/782a0bb82da9951bf641db1b290246945322ca36))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`0e4abe5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e4abe551515956485d6dd828019e67d56d1753d))
- Merge branches 'main' and 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`fa1af6d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fa1af6dd48cb7fb5b0907f2830ba2227fb326ba5))
- Merge pull request #2 from dstroy0/dependabot/github_actions/streetsidesoftware/cspell-action-8 ([`41f78d1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/41f78d18cd2f10caa4bbd5a89230bb85dc674b06))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`5c9e89c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5c9e89c149e1c302997b35934cd5345ba8c815f7))

### Features

- SNMP v1/v2c/v3 agent (USM authPriv) on the BER codec ([`1477bbd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1477bbdf6291ea37b0bac49be77b4b361945086e))
- HTTPS, web terminal, JSON helper, regex/interface routes, SNMP codec ([`69a864d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/69a864d4747872415a7b6b8ee3a99cf9481e87b2))

</details>

## [1.2.6] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.6 - 2026-06-23</b></summary>

### CI / Build

- update test report [skip ci] ([`666146e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/666146e250c43c883a2c00a9f706250f1ea610b1))
- update CHANGELOG.md [skip ci] ([`f873e5e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f873e5e8ac9d9f78cec57f73984c2e1d192df371))
- update CHANGELOG.md [skip ci] ([`d39f711`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d39f7110c93369c8315eb39b64b8b19553956a3d))

### Changes

- Bump version: 1.2.5 → 1.2.6 ([`2ae9c27`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ae9c2749d7d8589ab19df02ee775b7ae8ca228d))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`a0d83bd`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d83bd8386649367102d10e02c7d92bd5f0ceda))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`7b630b4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7b630b4278b0ade11bf92a4fb613b19e886d9956))

### Features

- path params, Digest auth, templating, middleware, and chunked responses ([`24adb9d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/24adb9df73d30c95f9b2d163014eece1c9131d9b))

</details>

## [1.2.5] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.5 - 2026-06-23</b></summary>

### CI / Build

- update test report [skip ci] ([`ee7322b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ee7322b0b21d6924c7e30d9e33dd8cd9bd8b9271))
- update CHANGELOG.md [skip ci] ([`b68d7e1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b68d7e1ac901fe4f9596913a5749075b32594d6f))
- update test report [skip ci] ([`075d4eb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/075d4eb03e1a5ec58df3941181cbe706a4175d0b))
- update CHANGELOG.md [skip ci] ([`2ee4a51`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2ee4a518f3c91adede0d3e7e77fd0afccd0787ac))
- update test report [skip ci] ([`89c466b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89c466b7f2c2678fbd6b303afe1f29e4f7f06a0b))
- update CHANGELOG.md [skip ci] ([`cc64a23`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cc64a234509ff4223a39b58425a4c6045a4cef3d))
- update test report [skip ci] ([`5a683e4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5a683e4314a21f818d6067b6a445212de4e8f572))
- update CHANGELOG.md [skip ci] ([`84c9c1d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/84c9c1dc8855b721bd4a3cd3602e87d213562492))
- update CHANGELOG.md [skip ci] ([`572b3e0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/572b3e0c461df3426dff20feb41d2e1ec6de6cb9))

### Changes

- Bump version: 1.2.4 → 1.2.5 ([`dfbcf71`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dfbcf7138c24edd7a5f96625b7994759d9c5e868))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c1ec861`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c1ec861b4aa06bd56ff5e15d3c0475b31974cce4))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f6557de`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f6557de195179e5f421ff62efa7820bcb5259385))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`d657132`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d65713242f3e2c8d339f27bdab765ce9cace0bc6))
- create squirty the injection squid; update docs ([`14d4ef3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14d4ef31d6cdf724f17f6f2e537a9d6a4292f85b))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`65fa7f4`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/65fa7f48d8e0dee67dccb4a6c6ff9b6f5c0485c8))
- update docs ([`7a5b461`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7a5b4618c003aeeba5353f25e482fb6b54cbd865))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`ea49289`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ea492898c6607d3d957b8ff30706bba37b9c31e4))

### Documentation

- docs formatting ([`9f10d21`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f10d211b21506d943790484d0c7ab3247bd6394))

### Features

- custom response headers/cookies + urlencoded form params ([`012829b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/012829b53fcfe261e836b3a9aa9d9496b583dc57))

</details>

## [1.2.4] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.4 - 2026-06-23</b></summary>

### Bug Fixes

- fix ci errors ([`8269a66`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8269a668527694c0801c938bc660498202ec390b))

### CI / Build

- update test report [skip ci] ([`4c39fa5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4c39fa58bb5f8766c14de5c63c2463165c82662f))
- update CHANGELOG.md [skip ci] ([`7cc3427`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7cc34279b71de4f5cb8dff8f61d480b032341a49))
- update test report [skip ci] ([`6ccd273`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6ccd2731089745ec3d533241f9dad8870bd7da80))
- update CHANGELOG.md [skip ci] ([`535d844`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/535d844998b6b895212a13ceff058410c2bcff81))
- update test report [skip ci] ([`a031108`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a031108933cfc6c4ff9c041a876c86c5660d9d09))
- update CHANGELOG.md [skip ci] ([`252fee7`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/252fee7c65779202acbfff5d019670f22ab9a6e9))
- update CHANGELOG.md [skip ci] ([`152a08d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/152a08df007aaf97722f0cfca676414e98cb3dc8))

### Changes

- Bump version: 1.2.3 → 1.2.4 ([`0344053`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0344053b3894e0e59643671f831e396969a82410))
- update readme ([`cbd487d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/cbd487dfa3554cfef7312e4f73221d24eb860ca9))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`08462d0`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/08462d02acaeb647f61d8678943a5f79e12a39f1))
- update todo ([`854d297`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/854d2972c63efa2979b0f5c9f144e0a18e903040))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`7c64a3e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7c64a3e50660aad1b3865d24018dfefd9292a88f))

</details>

## [1.2.3] - 2026-06-23

<details>
<summary><b>Show Changelog for version 1.2.3 - 2026-06-23</b></summary>

### Bug Fixes

- fix concurrency issue in test-report ([`7e1b38c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7e1b38c9e95a4594a1bc156efed63a09ee3732fe))

### CI / Build

- update test report [skip ci] ([`c5733da`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c5733da34beba9c03dd6ba92e148e48b6b98d175))
- update CHANGELOG.md [skip ci] ([`ef0de34`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef0de3447f92650254f63312ac9fd386799b5795))
- update test report [skip ci] ([`ac6c36f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ac6c36f39afc4c1be11de612e48e6b0700807da4))
- update CHANGELOG.md [skip ci] ([`996488a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/996488a133851861e9d61d4fa983b27ca169c25e))
- update test report [skip ci] ([`7f67f58`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7f67f58da64c63393d0ddc86bc35153ee03d46a5))
- update CHANGELOG.md [skip ci] ([`d10a5e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d10a5e8794b08e860237c987a893fcc1edf53da5))
- update test report [skip ci] ([`0e872ef`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e872ef5719e3a06adf7c57f79eed84ab125c80a))
- update CHANGELOG.md [skip ci] ([`b8ecc21`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b8ecc214ecb3d6da003c17f9b396dcd32bfc7cd7))
- update test report [skip ci] ([`d3bf700`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3bf7008af6e23784eb305e79d048af7bd5bfdfa))
- update CHANGELOG.md [skip ci] ([`e30b48d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e30b48d050fe6c44504fb30d639f0d7629a242a4))
- update test report [skip ci] ([`150f8bc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/150f8bcf8a053230f6a0c4705bc07ef1e6d18c91))
- update CHANGELOG.md [skip ci] ([`c20569a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c20569aec8ce215ba2e098b9337246126b79908b))
- update test report [skip ci] ([`1f2b41c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1f2b41c8954d206cdabaf1515d8de3aa9be0c863))
- update CHANGELOG.md [skip ci] ([`ad53276`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ad532769d8e980c83327d752194905a283bd81b8))
- update test report [skip ci] ([`ef9facc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ef9facc435b7caf534bb81e52b2bb938c8044c85))
- update CHANGELOG.md [skip ci] ([`206abe1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/206abe1a8f6a823af00b0aa93b7fb045dbe37b4f))
- update test report [skip ci] ([`b260304`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b2603045df12eeb2b14866398138f91a4f752fc3))
- update CHANGELOG.md [skip ci] ([`6863c57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6863c579e07218ebe2e431ee9bbfe814ac59efd4))
- update test report [skip ci] ([`604cef3`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/604cef3d8cd273916e04ed44ef95b94b1d9c96ee))
- update CHANGELOG.md [skip ci] ([`46d618d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/46d618d06f4e571b542a965e78a84d984940b0bf))
- bump patch version to 1.2.1 and update README ([`68d7375`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/68d73753852f61a8242bd1fcb795ce6d83ba19fc))
- update test report [skip ci] ([`8598435`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8598435a68f4ca6765cfc75f86a3528f766bef13))
- update CHANGELOG.md [skip ci] ([`4a2cd14`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a2cd14db09d426f7d71588fc6f790bd0f14df91))
- update test report [skip ci] ([`d356856`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3568563a7de39f27cd92ab9597a03e96116cf78))
- update CHANGELOG.md [skip ci] ([`b01f7b2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b01f7b20b3cad3f3eab4dab2909b6e3fd2d3f498))
- update test report [skip ci] ([`eb23a2d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eb23a2dceba5a93964791ecfc23b81166996f650))
- update CHANGELOG.md [skip ci] ([`63784f6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/63784f641ea0e54c4c6db7d4d0702307e3648225))
- update test report [skip ci] ([`4a536dc`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4a536dcbc54c8928f0daed6efdae7a12a376e620))
- update CHANGELOG.md [skip ci] ([`ce8e5fa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ce8e5facad2a2cbaa679799b99fd2f422211acbc))
- update test report [skip ci] ([`82b13cb`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/82b13cbdb044cd1a732d14b24acde31bb9f9c69a))
- update CHANGELOG.md [skip ci] ([`92583c6`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/92583c6ec6071110b5dd110ceac214ab26759f2e))
- update test report [skip ci] ([`0e20e9c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0e20e9c56db25b77d76ea34505d7f14a589f3dd8))
- update CHANGELOG.md [skip ci] ([`dfde0b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/dfde0b9f5369e1903071ee2806123fa646bfa948))
- update test report [skip ci] ([`ded48b9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ded48b99ed5d44b46bd27c93b4e23b4cc6e95a86))
- update CHANGELOG.md [skip ci] ([`a74c71d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a74c71d3c086e15abaa57ca6d29f6cec5da63717))
- update test report [skip ci] ([`0084249`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/00842490bae66c1eefb095445449af3a5e64eee6))
- update CHANGELOG.md [skip ci] ([`b5d7e33`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b5d7e330c022a097728917d9bada68548a276a62))
- update CHANGELOG.md [skip ci] ([`f306e1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f306e1ca909e9abd55db08632739509824d7a393))
- update CHANGELOG.md [skip ci] ([`66e3d57`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/66e3d57ce55bf678e5665bfc0e674a9af9bb0ccf))
- update CHANGELOG.md [skip ci] ([`484a46c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/484a46cfdda3148c6ade4ad02769b1132f9afb24))
- update CHANGELOG.md [skip ci] ([`655130c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/655130cc105c49bc2c724d1ced92c84483d453a9))
- update CHANGELOG.md [skip ci] ([`6e8beaa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6e8beaa140305d190bb46b884bf533c5155d6b52))
- update CHANGELOG.md [skip ci] ([`3a55a15`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3a55a1556faeaf96f31a400f5fff01012efdcf4e))
- update CHANGELOG.md [skip ci] ([`01d1e1a`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/01d1e1acbfc53b79de30074d6bc2bae6b4cb33ec))
- update CHANGELOG.md [skip ci] ([`a4d0a84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a4d0a849a95bc369ee567199f833fb50e34bd7e9))

### Changes

- Bump version: 1.2.2 → 1.2.3 ([`5225d85`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5225d85724d3dba79dde8bb2b2b091366bba33c5))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`3f3391b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3f3391b8329c04c3bc9a3174965b09a89bb75282))
- update initial search token ([`fba70f9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/fba70f924c0673c11bd52072276b6a9ab49260d2))
- Bump version: 1.2.1 → 1.2.2 ([`1cf6787`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1cf6787927782e495ce27ff4e0c8d0962a18efbc))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`e4d2614`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e4d26140b11b01c8e13ce8df1bb88d904be35c51))
- update bumpversion to use GPG ([`72686e8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72686e86c1ee7f23318575eb7906ace657702679))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`b8335ec`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b8335ec55cf53f07afd24b6cd85577d3d8d095cf))
- add version badge to docs ([`7ecebd1`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/7ecebd1d404c2bbe975d71d6f2760642b77b7eb8))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`aaa6b68`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/aaa6b68f47a5595214dd529ae3b7d83c15be8bcd))
- update initial search token ([`668f56e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/668f56e629e39a763991a29d86d461550c08d7eb))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f3a4a1c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f3a4a1c75084dc97a8f8c79d6e8ba1235eb5ed19))
- update initial search token ([`8a02843`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8a0284336640b3d194b2058530fd86b514b1668e))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`a65c596`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a65c5963d8bbf02dea7289c54e57a25647292ba0))
- update readme; add qol bumpversion ([`6062d0b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6062d0ba13d182fcbc1376a1636cd0382089ba5f))
- fix markdown formatting style check issues ([`d0399c2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d0399c23ca11bb2576ec1fdb6e484118be16a06a))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`566095c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/566095cc8065a74a9afe6b4e258628d1d15ea717))
- formatting ([`216902c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/216902c416b2bed37e5465073cdb05129ce35c2e))
-   - Add optional system services: ([`5b3c5f2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/5b3c5f2bb500d4d4eafbafb1650c4292514a558d))
- add toc ([`9f165c5`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9f165c53fedb4e40fddd532b4aaf2797010fe847))
- update md formatting; add collapsible sections ([`0b77665`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/0b776659877417ad63f6eaf141918194480b8201))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`89dcb6e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/89dcb6ee2fdab0adff07f5c954f2322059d4b612))
- update TEST_REPORT.md formatting ([`8f6891c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8f6891cd5a47d54023b39cc1d7983fde7d8f1b49))
- add md format linter workflow ([`06e965b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/06e965b166e5ceccbf5677aa66c9e8eef8732a8f))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c653ca9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c653ca91f0deab888bef98f8f3291b20cfa8b34e))
- update clang-format workflow to use v22 ([`6654bfa`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/6654bfa5d0805d2a480c28f33c4dd33d28ab620b))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`f833120`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f833120b824fda7e9c1ea622b786b7f7f7ef0549))
- stop tracking .vscode/ ([`b618929`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b61892970a93f631aac292731c55d4e589b61dd6))
- format ([`290f745`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/290f74509db99120b5f6febbb4cbb175aab72f8f))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c4de884`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c4de884402685b2aea7a5711926d832a8ab90868))
- format codebase; check todo list for the current state of the library before the version bump finalizations ([`2cc3601`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2cc3601a965cacb7036f1eafd80be5bfc7ff10dc))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`e04a03b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e04a03b8d0a1feb524cd7ab8d9119066df54a628))
- update examples; verify crypto logic by hand; second pass optimization complete; begin TODO feature implementation, then add new bugs to TODO, then squash bugs before third pass optimization ([`4e82975`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4e829757713b3d893804d5ef917db5e269cbff75))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c214285`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c21428503d9f8f0e4e3acd035b2dc9fe598587ff))
- patch ([`e689c81`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e689c81d911c0be76c1e85c989d178aff3f2ecd1))
- add support for Telnet and partially implement SSH; add port listener abstraction layer; add more hw crypto; update test suite to account for new functionality; reorganize network_drivers/, it has subfolders for all OSI layers, functionally grouped by layer; lint codebase; spellcheck codebase; move test results to test/TEST_REPORTS.md; create test/TEST_DOCUMENTATION.md and copy all test documentation to it;, link to test/TEST_DOCUMENTATION.md and test/TEST_REPORTS.md in the README.md; create RFC.md; move RFC info from README.md; remove RFC info from the README.md; link to RFC.md in README.md; ([`75ab65e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/75ab65ef84321dbc824b25e1a4f240f5ad56f1f2))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`c8c043c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/c8c043c319e8c2967e3939d607aff6056e51eee9))

### Documentation

- move detailed README body to docs/README.md ([`8eb8de2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/8eb8de2105b1590bff3736a09dbd13ff54b81330))
- add GitHub Actions workflow status badges to README.md ([`346c0cf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/346c0cf9362244e711d945a01abc0ba9ddee5203))
- update feature flags info & add provisioning, MDNS & NTP services ([`b4135ba`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/b4135ba256608624ac5037f5f9e694775f4dca20))
- docs ([`93c7ef8`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/93c7ef88637ee6576bef7a6ba34a358881ff42a3))

### Refactor

- centralize HTML templates into DETWS_HTML.h ([`72f5d1f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/72f5d1f8b83fbfd2b2d6593ba76a9fa391964f8b))

</details>

## [1.2.0] - 2026-06-20

<details>
<summary><b>Show Changelog for version 1.2.0 - 2026-06-20</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`388646e`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/388646e7d98a3d23f6e0f959ea03b87e2234bd4d))
- update CHANGELOG.md [skip ci] ([`399b835`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/399b835a8378ae3d5c5079a2078ae9f2c152570a))

### Changes

- add features; bump version ([`a0d5a67`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/a0d5a67dbeaed2cfa832d8b7d7f9b80f0494dcc1))
- implement websockets, sse, auth per-route, multipart form parsing, hw SHA-1 (mbedtls/sha1), user selectable features/configuration (e.g. ws, sse, etc.) via flag and compile-time constants. ([`d3c8ec9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d3c8ec97acf3ec66bcbe103128e944fcde1ab2ed))
- update version to 1.1.0, add author and maintainer info, and update description in library.properties ([`4da247c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/4da247cea4209809b1d939dd85318f7539192f77))

</details>

## [1.1.0] - 2026-06-20

<details>
<summary><b>Show Changelog for version 1.1.0 - 2026-06-20</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`bb32fdf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/bb32fdfba90eb4aeb4da4fd6130bae0ebc414813))

### Changes

- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`86988ca`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/86988ca73c51b903d15034deccf533e4c504e8a0))
- format codebase using .clang-format, lint and add more examples, lint docs ([`12baf8d`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/12baf8d5ce6976ceef0a65b2309702dfd1babf89))

</details>

## [0.1.0] - 2026-06-20

<details>
<summary><b>Show Changelog for version 0.1.0 - 2026-06-20</b></summary>

### CI / Build

- update CHANGELOG.md [skip ci] ([`3db5a62`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3db5a6212dafab7c44dd6a68915aaa33e957f0f3))
- update CHANGELOG.md [skip ci] ([`3ddf27c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/3ddf27c9362bf869f0216afc19874d141fbb0e24))
- update CHANGELOG.md [skip ci] ([`eeea908`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/eeea9082c24ae05b8898ed371cef5dbffb0386db))
- update CHANGELOG.md [skip ci] ([`e81b25f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/e81b25ff412562043e086ffc315852f16ce3f180))
- update CHANGELOG.md [skip ci] ([`1a4c39b`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1a4c39be56b8ba74da376b60f4b45bbb6ac57f07))
- update CHANGELOG.md [skip ci] ([`f04ac84`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/f04ac84bdb7e675a6d2b255a87372bfa91ffd048))
- ci initial commit ([`939e034`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/939e0349f40368dc9a7b580f00d8424b24a10258))

### Changes

- update test suite, adjust logic for RFC compliance ([`689b240`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/689b24052bbbf8ba1905697b845e55af1de07c1d))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`73d5f25`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/73d5f2525e6756cdb9762b8477c43d36fcdb0215))
- add dependabot github-actions dependency auto-updater ([`787158f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/787158f7214963b54bf77cb7f93f78315764b9d2))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`99a92ee`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/99a92ee1827c72ce691801a1d52612ffdb3b5c71))
- update ci action versions ([`14748bf`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/14748bfcb83c501975b5e6b8f50559e0e8f66513))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`16039a9`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/16039a9384c580cf02392e25862f10cd45ac5664))
- add examples; update README ([`ecfeb0f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/ecfeb0faa559fbc674671a41a57e2ed03e10ecf8))
- Merge branch 'main' of https://github.com/dstroy0/DeterministicESPAsyncWebServer ([`846720c`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/846720cc8111f113b5af11931eb17d9a81ef1c11))
- update cliff.toml for this repo ([`1fb43ae`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/1fb43ae1229ec4391ec7c89a1b9d9747faf327ce))
- update CHANGELOG output dir ([`2838da2`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/2838da22b3673aef7f6b365a1e0ec2c3ebedd504))
- update CHANGELOG output dir ([`d799b4f`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/d799b4f7c0031f5e539c9f3544127f283416f87e))
- initial commit ([`9b48742`](https://github.com/dstroy0/DeterministicESPAsyncWebServer/commit/9b48742e971a4e41924df8d67aab91f10c3346e8))

</details>
