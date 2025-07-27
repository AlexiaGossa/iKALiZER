#ifdef _WIN32
#pragma warning( disable : 4731 )
#endif

unsigned long ca_crypt_lut[514] = {
0xdbc09577, 0x7a5cf72c, 0xece675d1, 0xfc8f8cbb, 0x5bf6c7dd, 0xd660ce2f, 0xf7d10567, 0x13b19376, 
0xbad7de19, 0xc72fec88, 0x41ab9930, 0xcb16f393, 0x3f53d648, 0x3683af22, 0xc07b2a4a, 0x4ab8f6e8, 
0x6de92a26, 0x768604e4, 0x2ecef459, 0x67204a53, 0xd0028d24, 0x2fe52048, 0x471e5012, 0x95d98169, 
0x45836fba, 0x0ed38066, 0xdb5e66b8, 0x4c6e9308, 0xf7fe0d1d, 0x7a7103e9, 0x9c96df27, 0x8ea8b196, 
0x5ba7015c, 0x0b6cc281, 0xf26a8b70, 0x8d7149a7, 0x8e7432a7, 0x7deab511, 0x84ab5be2, 0x3da5a64a, 
0x63cf2ac6, 0xd50d9fec, 0x3a2e4fdc, 0xd79f7a0f, 0x35be86e7, 0x4724c710, 0x7186234f, 0x9fc5e7f1, 
0x9361f174, 0x4fe175c1, 0x28ea883e, 0x42ceb4d8, 0x8750d025, 0xaa9fa39f, 0x19aada0c, 0x55aa546b, 
0xa8b7679d, 0x9c17ebbf, 0xaa35c695, 0xd554ef49, 0x4b33a3cb, 0xeebe1731, 0x5eab7950, 0x838d166c, 
0x4c11ebee, 0x9c9043d5, 0xd4563aa2, 0x8be7ce09, 0x31a3c1e9, 0x64786ce1, 0x502945cc, 0x93db30a3, 
0x85cdac7e, 0xd76a7988, 0xc470b767, 0x94b44650, 0x3b4017e4, 0x99d7de6c, 0x9e27f633, 0x04de566e, 
0xb783539f, 0xa2137fcd, 0x107ba2a2, 0x167a8a4e, 0x23d9dcfd, 0x149ae477, 0xdc76c17e, 0x2c69533a, 
0xf092900c, 0x7c0bbdc6, 0xd1f4bb7d, 0x5b7bdffb, 0x96370fb5, 0x61e064a1, 0xde3997bd, 0x09169365, 
0x98f3639e, 0x542c770f, 0x95ae5f2d, 0x27abd097, 0x69f058d7, 0x54aecc5c, 0x9110f493, 0xec85c41b, 
0x45c40faf, 0x8104d567, 0x5e0e9ae2, 0xc650fe89, 0x9935a6c1, 0x7dbcf177, 0x731179e6, 0xba90604b, 
0x827b5a3c, 0xc474a141, 0x1f908a27, 0x7a3edf8c, 0xbfe5bb60, 0xa93e9559, 0xb12c559d, 0xc9c088f9, 
0xe401208d, 0xbd64fdc2, 0xc82f28db, 0x2ff5edb7, 0x24f0152c, 0xb60e3f64, 0xf8ac8d36, 0x7f66e484, 
0xda2b8627, 0x09172108, 0x1c01afe5, 0x4986b3c4, 0x68931f42, 0xfb3ff945, 0x3e56b049, 0xbf609f3b, 
0xa1a3613d, 0xe695db9c, 0x97049050, 0xc193ac4a, 0xecc64d14, 0xc4ad1390, 0x9203aee0, 0x64cf6ad3, 
0x337ebabb, 0x50a142f6, 0x411c2ffb, 0x21ccb502, 0x99f0d7f8, 0xe6d1b44c, 0x3f3457a8, 0xba378bf0, 
0x2df4a0c6, 0x462c0424, 0xaa863f42, 0xe59fcdf5, 0x295eff39, 0xbfd38d86, 0x9860da01, 0x04e05c33, 
0x715a1607, 0x24cddd07, 0xc38ae920, 0x029e94ea, 0xd0afe137, 0x52faad54, 0xcdc24a05, 0x02dc789f, 
0xacce84b7, 0xc1ff207d, 0x61a3a844, 0x3590735f, 0xe578a9d1, 0x20e89624, 0x2abd5709, 0xd781003c, 
0x5398cd39, 0x73b0aa8c, 0x96a4d84f, 0x4435a737, 0x1e005ca2, 0x470fee5f, 0xef05c267, 0x02de7c47, 
0xb5034a93, 0xd22e2ca4, 0x65ac1b86, 0x4907ba9a, 0x2d71b643, 0xb404686f, 0x1e220d25, 0x5b3481cc, 
0x9d04ecbe, 0x311b3917, 0x10df3525, 0xc988b3fe, 0x2f816101, 0xd40ee04b, 0x6a2425b2, 0x516f6e67, 
0x1552718e, 0x946c2a5f, 0x2f712e2f, 0x5038da1f, 0x71c3a4d0, 0x718a6b48, 0x70091d04, 0x0dedd49b, 
0x777747ed, 0x6ff20fe3, 0xb052a15f, 0x97703ce3, 0x2e300e6e, 0x4c573e52, 0xd0d0dde7, 0x41f1e888, 
0x158ad201, 0x74fde05e, 0x1a5f33f0, 0x4a908b17, 0xe7530c29, 0x31761d66, 0x4b57b01f, 0x02ec1f48, 
0x833bc47e, 0x2d525726, 0xe9a05d99, 0x7f6a88fa, 0x6c338ceb, 0xdee5eb88, 0x1952df9a, 0x884a0579, 
0x38abbd0a, 0x0462ca30, 0x143250f3, 0x84c1765f, 0x29bee42c, 0x51e181b6, 0xffdaf1d2, 0xdf8e4ca1, 
0xdd97ff44, 0xa8471f50, 0xdb9bc964, 0x0ff646e1, 0xaa9c6ee2, 0xa6205955, 0x873ebc76, 0x26a5ac80, 
0xdf82a45d, 0xfd1a463a, 0x7cd9fc89, 0x9d6a3056, 0xee37e697, 0xb18cb5be, 0x4cb8d235, 0x84eb56b5,
0xe4aeffad, 0x65acd6ea, 0x29f0a8c0, 0x4364391b, 0xd49f5b89, 0x7e198bff, 0xf745904a, 0xa8e416e0, 
0x0d691394, 0xd5f226ea, 0x499c9ade, 0x18460649, 0x00cbfad1, 0x3a7aa99b, 0xac4fd2a2, 0x5660705e, 
0xf745b866, 0x917af895, 0xb4858f16, 0x4a4cd6bb, 0x248af6d1, 0x3be6d9dc, 0x5291594f, 0xb6582082, 
0x63119f47, 0x70124d99, 0x1355738e, 0x9665e800, 0xa99bad93, 0x3d4e611e, 0xe6c9fd5a, 0x8af6b3d6, 
0x5fa6ecbf, 0x1be7c5c1, 0xc0ab6765, 0xa4bd629a, 0x94701b29, 0x6608c029, 0x5e2d63f8, 0x1017cf0a, 
0xb2d2c719, 0x0fdaa23e, 0xb322df79, 0x7b8f14d9, 0x9c7d1639, 0xfde133a1, 0xf69794c0, 0xc1451036, 
0xc368e7eb, 0x94958f5e, 0x9780ca7b, 0xad5f1d79, 0x4863acfa, 0x55aff8c8, 0xc1f87be3, 0x802151a3, 
0x7182bb13, 0xa8e90d47, 0x4fc525c8, 0x0faff3f2, 0x0fe3e0fb, 0xfdff4c00, 0x51f30ae5, 0xc34a8b4d, 
0xda7c85a7, 0x23f6de42, 0x2e6eec06, 0x8356d009, 0x6bcdb88c, 0xdd0ab8fe, 0xc51a1a7e, 0xc83d2523, 
0x374a89cb, 0x650c2258, 0x56bb9eb9, 0xf02ffcf1, 0x133738c1, 0x5b8ae1be, 0x1dc73266, 0xc415f1d0, 
0xcd7f445e, 0xe7c14cce, 0xb1ab74c7, 0xb09246f7, 0x90bdf2fc, 0x09f5fb40, 0xd347ed6f, 0xdb688386, 
0xa9864f8d, 0x43507650, 0x216588e0, 0x266cf7ef, 0x4c7451ed, 0x2330c0d1, 0xaaed8de0, 0xd1948edb, 
0xafe52219, 0x607b3278, 0x85a7b8e7, 0x8bf725e8, 0x43606543, 0x16b48273, 0x138513c5, 0x247ae205, 
0x9cf6bc99, 0x3b9b3c2d, 0x10e3e7b8, 0xcdea26f7, 0x22bcc4c8, 0xdb3d45e9, 0xed5eb2bc, 0xd59a3bae, 
0xaf6bfffd, 0xecc51b22, 0x8e200761, 0xefceb1aa, 0x7e538829, 0xa0eee664, 0xfbcaa90e, 0x285c3894, 
0xc3eee7c4, 0xa6db5af6, 0x0d701912, 0x680e0405, 0x614540d3, 0xe1eac391, 0x791cba8f, 0x1516904e, 
0x79f1a1f5, 0xa5c1d54c, 0x88e018e3, 0xb1c2f905, 0xd69a2346, 0x6618e2f0, 0x7fc17629, 0xad9937da, 
0xd1b33687, 0x63ef4c95, 0x39eaf145, 0x6b1d1587, 0x5a03c3ac, 0xe1f5d20b, 0x0d08b158, 0xc9e2d4c2, 
0xe6c6d9f0, 0xe63d869f, 0xaddc58f7, 0x08eced49, 0x14520191, 0x153ce099, 0x1c43ba6a, 0x5984377c, 
0x4e80c82e, 0xfe7300d7, 0x3ccd2210, 0x4193a55a, 0x9aa7de7a, 0x975467fd, 0x11315af0, 0x24d06ac4, 
0xd5108252, 0xa491d07b, 0xeade49d7, 0xd6f89e8f, 0x1322b88a, 0x9e60eaef, 0xec3357cf, 0x149e4269, 
0xfccb01e1, 0x55fc9c30, 0x4a8ca36f, 0xb38f5dc6, 0x2a6161f4, 0x345521eb, 0x11337943, 0x9f9dd358, 
0x6354a354, 0x4d0c7b91, 0x9b74e150, 0xe6420f35, 0x5fdc9646, 0x9aa544af, 0xbf97a5cd, 0xc3d54b4c, 
0xce61162a, 0x3fde4708, 0x64abf4e6, 0x78a05f66, 0x6a086681, 0x9dc28257, 0x07c87765, 0x681cd288, 
0xee126ca0, 0xe997ed7e, 0xf1ca77d6, 0x3819e029, 0x6921205e, 0x87a74566, 0xa80a7b5a, 0xe4dd8d9c, 
0x647c3c00, 0xe53db969, 0x324cf514, 0xacd1cfab, 0x7026b71b, 0x9243ced7, 0xa7140d10, 0xcb390015, 
0x01d7129e, 0x1183c38c, 0xa2b5eb26, 0x179b481c, 0xbdefa2eb, 0xdfc1ec1f, 0xba111d49, 0x5c555415, 
0x2493b0ea, 0x0534b02f, 0x38bd0bbd, 0x1095444a, 0x9ab0c4e2, 0xbe32defa, 0x27e18661, 0x797ab238, 
0x1bb4b895, 0xc27e9daf, 0x309d5e0d, 0xa472d9b9, 0x03777c5f, 0x3e38495c, 0x80eadb3c, 0x96348b6d, 
0xb87b75f4, 0xa96d4d7a, 0xf6a440ff, 0x52e6b762, 0xeb9d4e8b, 0xdc7df1ff, 0xa7bd72d5, 0xf3aa0a15, 
0xe2520bc1, 0xdcc08f9e, 0xcc8d0f6d, 0x726c81be, 0x29a0130e, 0x93ddc424, 0x8321c8a6, 0x34118b9f, 
0xbf7a0d58, 0x3188cd91, 0x59f87f7b, 0xe607af61, 0x171027aa, 0x9e2668ca, 0x56965855, 0x3af3f93d,
0x1e376c08, 0x5141ab53 };

unsigned long __ca_crypt_sha512_k[160] =
            {
0x428a2f98, 0xd728ae22, 0x71374491, 0x23ef65cd, 0xb5c0fbcf, 0xec4d3b2f, 0xe9b5dba5, 0x8189dbbc,
0x3956c25b, 0xf348b538, 0x59f111f1, 0xb605d019, 0x923f82a4, 0xaf194f9b, 0xab1c5ed5, 0xda6d8118,
0xd807aa98, 0xa3030242, 0x12835b01, 0x45706fbe, 0x243185be, 0x4ee4b28c, 0x550c7dc3, 0xd5ffb4e2,
0x72be5d74, 0xf27b896f, 0x80deb1fe, 0x3b1696b1, 0x9bdc06a7, 0x25c71235, 0xc19bf174, 0xcf692694,
0xe49b69c1, 0x9ef14ad2, 0xefbe4786, 0x384f25e3, 0x0fc19dc6, 0x8b8cd5b5, 0x240ca1cc, 0x77ac9c65,
0x2de92c6f, 0x592b0275, 0x4a7484aa, 0x6ea6e483, 0x5cb0a9dc, 0xbd41fbd4, 0x76f988da, 0x831153b5,
0x983e5152, 0xee66dfab, 0xa831c66d, 0x2db43210, 0xb00327c8, 0x98fb213f, 0xbf597fc7, 0xbeef0ee4,
0xc6e00bf3, 0x3da88fc2, 0xd5a79147, 0x930aa725, 0x06ca6351, 0xe003826f, 0x14292967, 0x0a0e6e70,
0x27b70a85, 0x46d22ffc, 0x2e1b2138, 0x5c26c926, 0x4d2c6dfc, 0x5ac42aed, 0x53380d13, 0x9d95b3df,
0x650a7354, 0x8baf63de, 0x766a0abb, 0x3c77b2a8, 0x81c2c92e, 0x47edaee6, 0x92722c85, 0x1482353b,
0xa2bfe8a1, 0x4cf10364, 0xa81a664b, 0xbc423001, 0xc24b8b70, 0xd0f89791, 0xc76c51a3, 0x0654be30,
0xd192e819, 0xd6ef5218, 0xd6990624, 0x5565a910, 0xf40e3585, 0x5771202a, 0x106aa070, 0x32bbd1b8,
0x19a4c116, 0xb8d2d0c8, 0x1e376c08, 0x5141ab53, 0x2748774c, 0xdf8eeb99, 0x34b0bcb5, 0xe19b48a8,
0x391c0cb3, 0xc5c95a63, 0x4ed8aa4a, 0xe3418acb, 0x5b9cca4f, 0x7763e373, 0x682e6ff3, 0xd6b2b8a3,
0x748f82ee, 0x5defb2fc, 0x78a5636f, 0x43172f60, 0x84c87814, 0xa1f0ab72, 0x8cc70208, 0x1a6439ec,
0x90befffa, 0x23631e28, 0xa4506ceb, 0xde82bde9, 0xbef9a3f7, 0xb2c67915, 0xc67178f2, 0xe372532b,
0xca273ece, 0xea26619c, 0xd186b8c7, 0x21c0c207, 0xeada7dd6, 0xcde0eb1e, 0xf57d4f7f, 0xee6ed178,
0x06f067aa, 0x72176fba, 0x0a637dc5, 0xa2c898a6, 0x113f9804, 0xbef90dae, 0x1b710b35, 0x131c471b,
0x28db77f5, 0x23047d84, 0x32caab7b, 0x40c72493, 0x3c9ebe0a, 0x15c9bebc, 0x431d67c4, 0x9c100d4c,
0x4cc5d4be, 0xcb3e42b6, 0x597f299c, 0xfc657e2a, 0x5fcb6fab, 0x3ad6faec, 0x6c44198c, 0x4a475817 };


void CA_Encrypt64 ( unsigned char * pData, long lBlocCount, unsigned long dwLowKey, unsigned long dwHighKey )
{
    __asm {    
                mov         edi, pData
                mov         esi, lBlocCount
                mov         ecx, dwLowKey
                mov         edx, dwHighKey
                push        ebp
                
                push        ecx
                push        edx
                
                mov         ebp, ecx
                xor         ebp, edx
                
                mov         eax, ecx
                mov         ebx, edx

                shr         eax, 7                
                and         eax, 2047
                shr         ebx, 17
                and         ebx, 2047
                
                xor         ecx, ca_crypt_lut[ebx]
                xor         edx, ca_crypt_lut[eax]
                
                mov         eax, ecx
                mov         ebx, edx
                
                shr         eax, 1
                shr         ebx, 1
                
                and         ecx, 1
                and         edx, 1
                
                neg         ecx
                neg         edx
                
                and         ecx, 0xd0000001
                and         edx, 0x40000061
                
                xor         ecx, eax
                xor         edx, ebx
                
            _loop:
    
                //Chargement de 64 bits
                mov         eax, [edi]
                mov         ebx, [edi+4]
                
                //Cryptage : XOR
                xor         eax, ecx
                xor         ebx, edx
                
                mov         [edi],   eax
                mov         [edi+4], ebx
                
                //Modification de la clef de cryptage
                xor         ebp, eax
                add         ebp, ebx
                and         ebp, 511
                xor         ecx, __ca_crypt_sha512_k[ebp]
                
                //lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xd0000001u); /* taps 32 31 29 1 */
                mov         eax, ecx
                mov         ebx, edx
                
                shr         eax, 1
                shr         ebx, 1
                
                and         ecx, 1
                and         edx, 1
                
                neg         ecx
                neg         edx
                
                and         ecx, 0xd0000001
                and         edx, 0x40000061
                
                xor         ecx, eax
                xor         edx, ebx
                
                xchg        ecx, edx
                
                add         edi, 8
                
                xor         ecx, [esp+4]
                xor         edx, [esp]
                
                dec         esi
                jnz         _loop
                
                pop         ecx
                pop         edx
                pop         ebp
    }
}

void CA_Decrypt64 ( unsigned char * pData, long lBlocCount, unsigned long dwLowKey, unsigned long dwHighKey )
{
    __asm {    
                mov         edi, pData
                mov         esi, lBlocCount
                mov         ecx, dwLowKey
                mov         edx, dwHighKey
                push        ebp
                
                push        ecx
                push        edx
                
                mov         ebp, ecx
                xor         ebp, edx
                
                mov         eax, ecx
                mov         ebx, edx

                shr         eax, 7                
                and         eax, 2047
                shr         ebx, 17
                and         ebx, 2047
                
                xor         ecx, ca_crypt_lut[ebx]
                xor         edx, ca_crypt_lut[eax]
                
                mov         eax, ecx
                mov         ebx, edx
                
                shr         eax, 1
                shr         ebx, 1
                
                and         ecx, 1
                and         edx, 1
                
                neg         ecx
                neg         edx
                
                and         ecx, 0xd0000001
                and         edx, 0x40000061
                
                xor         ecx, eax
                xor         edx, ebx
                
            _loop:
    
                //Chargement de 64 bits
                mov         eax, [edi]
                mov         ebx, [edi+4]
                
                xor         ebp, eax
                add         ebp, ebx
                
                //Cryptage : XOR
                xor         eax, ecx
                xor         ebx, edx
                
                mov         [edi],   eax
                mov         [edi+4], ebx
                
                //Modification de la clef de cryptage
                and         ebp, 511
                xor         ecx, __ca_crypt_sha512_k[ebp]
                
                //lfsr = (lfsr >> 1) ^ (-(lfsr & 1u) & 0xd0000001u); /* taps 32 31 29 1 */
                mov         eax, ecx
                mov         ebx, edx
                
                shr         eax, 1
                shr         ebx, 1
                
                and         ecx, 1
                and         edx, 1
                
                neg         ecx
                neg         edx
                
                and         ecx, 0xd0000001
                and         edx, 0x40000061
                
                xor         ecx, eax
                xor         edx, ebx
                
                xchg        ecx, edx
                
                add         edi, 8
                
                xor         ecx, [esp+4]
                xor         edx, [esp]
                
                dec         esi
                jnz         _loop
                
                pop         ecx
                pop         edx
                pop         ebp
    }
}


