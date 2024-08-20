var group___model_time_sr =
[
    [ "mmdlTimeStatusParam_t", "group___model_time_sr.html#structmmdl_time_status_param__t", [
      [ "subSecond", "group___model_time_sr.html#aaf48025be839c5cc12c6c6704bd2478f", null ],
      [ "taiSeconds", "group___model_time_sr.html#a730ab7c9bf554ea71be1189751f9283d", null ],
      [ "taiUtcDelta", "group___model_time_sr.html#a662441e27b02a498789bb47dcbc46175", null ],
      [ "timeAuthority", "group___model_time_sr.html#aa5b9ddce6a40e8b4850266afbb709bb4", null ],
      [ "timeZoneOffset", "group___model_time_sr.html#aaa81c60e3d82b53965dfaca410480632", null ],
      [ "uncertainty", "group___model_time_sr.html#aca76149653c0cd02785b69e7bab7575d", null ]
    ] ],
    [ "mmdlTimeZoneStatusParam_t", "group___model_time_sr.html#structmmdl_time_zone_status_param__t", [
      [ "currentOffset", "group___model_time_sr.html#ad7313ed0ced79a7a6c30ff9540d89643", null ],
      [ "newOffset", "group___model_time_sr.html#a768b4bc6a5a097a1a13c1d90c9337f97", null ],
      [ "taiOfZoneChange", "group___model_time_sr.html#a6a175f6c0197fa5b2b77107ecd15c98a", null ]
    ] ],
    [ "mmdlTimeRoleStatusParam_t", "group___model_time_sr.html#structmmdl_time_role_status_param__t", [
      [ "timeRole", "group___model_time_sr.html#a1408ede67be03a1b7933dfa25f568b6a", null ]
    ] ],
    [ "mmdlTimeStates_t", "group___model_time_sr.html#unionmmdl_time_states__t", [
      [ "timeDeltaState", "group___model_time_sr.html#a1a78eebb6cf7fb5f6b5de1ea6fe8db06", null ],
      [ "timeRoleState", "group___model_time_sr.html#a24f0819196da3896738f92b2751f4fb9", null ],
      [ "timeState", "group___model_time_sr.html#a83182ff3947489dd185363a272c4c35a", null ],
      [ "timeZoneState", "group___model_time_sr.html#aae8b96362668c32a63f03db40060b336", null ]
    ] ],
    [ "mmdlTimeSrStateUpdate_t", "group___model_time_sr.html#structmmdl_time_sr_state_update__t", [
      [ "elemId", "group___model_time_sr.html#a6f284f52dd638be12f8be1efbd280060", null ],
      [ "hdr", "group___model_time_sr.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "state", "group___model_time_sr.html#af1c0cb95b23f891ac1d648b6f5614d47", null ],
      [ "stateUpdateSource", "group___model_time_sr.html#a4772d435b970961840de3380bf055ee4", null ]
    ] ],
    [ "mmdlTimeSrCurrentState_t", "group___model_time_sr.html#structmmdl_time_sr_current_state__t", [
      [ "elemId", "group___model_time_sr.html#a6f284f52dd638be12f8be1efbd280060", null ],
      [ "hdr", "group___model_time_sr.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "state", "group___model_time_sr.html#af1c0cb95b23f891ac1d648b6f5614d47", null ]
    ] ],
    [ "mmdlTimeSrEvent_t", "group___model_time_sr.html#unionmmdl_time_sr_event__t", [
      [ "currentStateEvent", "group___model_time_sr.html#adcd8a4b0779c74c85ade8fe437824cd1", null ],
      [ "hdr", "group___model_time_sr.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "statusEvent", "group___model_time_sr.html#aa8f94240c572b9afc9ec9444c9025aad", null ]
    ] ],
    [ "mmdlTimeSrDesc_t", "group___model_time_sr.html#structmmdl_time_sr_desc__t", [
      [ "initialState", "group___model_time_sr.html#a726c8c065c80fea02d00c22ddfad8b1f", null ],
      [ "storedTimeDeltaState", "group___model_time_sr.html#ab873a4118e3bc2566f5b8a9ff0617542", null ],
      [ "storedTimeRoleState", "group___model_time_sr.html#a98a907f170339b68f81070de45445656", null ],
      [ "storedTimeState", "group___model_time_sr.html#af4ee020c3d839b426825a4221a267a44", null ],
      [ "storedTimeZoneState", "group___model_time_sr.html#aa09ef0808c05a8678f9c6f29bdc03880", null ]
    ] ],
    [ "MmdlTimeDeltaSrGetState", "group___model_time_sr.html#ga9de3ef3029bfebbafede7dcf0e4478e7", null ],
    [ "MmdlTimeDeltaSrSetState", "group___model_time_sr.html#ga9877872dca61420ec3c05c30331fa79c", null ],
    [ "MmdlTimeSetupSrDeltaGetState", "group___model_time_sr.html#ga6008e8dc3e6d0d706f0354a465ecadde", null ],
    [ "MmdlTimeSetupSrDeltaSetState", "group___model_time_sr.html#gac288fcdf3693775c01a42bf54e1b2da5", null ],
    [ "MmdlTimeSetupSrGetState", "group___model_time_sr.html#ga17b3811797cbf378ffd921ef6895b7e7", null ],
    [ "MmdlTimeSetupSrHandler", "group___model_time_sr.html#gaa74554e65a9f700d469dbb9251a3a01c", null ],
    [ "MmdlTimeSetupSrHandlerInit", "group___model_time_sr.html#ga94f4c065eec6964ce52ed6fd94bc386e", null ],
    [ "MmdlTimeSetupSrInit", "group___model_time_sr.html#ga165029613fcd343a18ba74744ace788e", null ],
    [ "MmdlTimeSetupSrRegister", "group___model_time_sr.html#gaf0a81afd3544fdea5a53c70a4ed781e9", null ],
    [ "MmdlTimeSetupSrRoleGetState", "group___model_time_sr.html#ga88480d4c06330642210f69a67a5204e4", null ],
    [ "MmdlTimeSetupSrRoleSetState", "group___model_time_sr.html#gab337feb252851ac9fe91b3a1dd69f8d9", null ],
    [ "MmdlTimeSetupSrSetState", "group___model_time_sr.html#ga9b160ee2270816ac4000ba87ef9ea6cc", null ],
    [ "MmdlTimeSetupSrZoneGetState", "group___model_time_sr.html#gad3d9481f2898aeeb136d1872a17c4642", null ],
    [ "MmdlTimeSetupSrZoneSetState", "group___model_time_sr.html#ga055696daceded98742efbfd553dd2884", null ],
    [ "MmdlTimeSrGetState", "group___model_time_sr.html#ga3e79ce51f4ebdd5defac5fd17f3abb6f", null ],
    [ "MmdlTimeSrHandler", "group___model_time_sr.html#ga365e616a3b797224fb209418cec8240b", null ],
    [ "MmdlTimeSrHandlerInit", "group___model_time_sr.html#ga7e2deb9f5494597589026a48f0ffb3c7", null ],
    [ "MmdlTimeSrInit", "group___model_time_sr.html#ga934cac0af50622b90aed288a98ae2d28", null ],
    [ "MmdlTimeSrPublish", "group___model_time_sr.html#ga5d257591b615a7301cdd1d19dbac3f2a", null ],
    [ "MmdlTimeSrRegister", "group___model_time_sr.html#ga4334e3ce551d6b64d726fb902c39e767", null ],
    [ "MmdlTimeSrSetState", "group___model_time_sr.html#ga66c4c635489791dd50f76b844b9dee55", null ],
    [ "MmdlTimeSrZoneGetState", "group___model_time_sr.html#gae53d2192eaa1129d5175386416a88582", null ],
    [ "MmdlTimeSrZoneSetState", "group___model_time_sr.html#gaacb2fe6fdce7917e97f33d855744a5e0", null ],
    [ "mmdlTimeSetupSrHandlerId", "group___model_time_sr.html#gaf6501ba2577f69525eddcf79c99c8f53", null ],
    [ "mmdlTimeSetupSrRcvdOpcodes", "group___model_time_sr.html#gab5c823f4051b5bbb031665f1eb9c551f", null ],
    [ "mmdlTimeSrHandlerId", "group___model_time_sr.html#ga961148d670fe37af5ec5454cac88f49f", null ],
    [ "mmdlTimeSrRcvdOpcodes", "group___model_time_sr.html#ga7ffbf63fc81584760e0929ce05dcdc9c", null ]
];