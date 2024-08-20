var group___model_time_cl =
[
    [ "mmdlTimeSetParam_t", "group___model_time_cl.html#structmmdl_time_set_param__t", [
      [ "state", "group___model_time_cl.html#a82716c2da19cceb7e3ece79fadc23de8", null ]
    ] ],
    [ "mmdlTimeZoneSetParam_t", "group___model_time_cl.html#structmmdl_time_zone_set_param__t", [
      [ "state", "group___model_time_cl.html#a4e07ccfa9dadc9d9f8b22d1f003dccb6", null ]
    ] ],
    [ "mmdlTimeDeltaSetParam_t", "group___model_time_cl.html#structmmdl_time_delta_set_param__t", [
      [ "state", "group___model_time_cl.html#a904cd9393d7a41935764e2806fc5907f", null ]
    ] ],
    [ "mmdlTimeRoleSetParam_t", "group___model_time_cl.html#structmmdl_time_role_set_param__t", [
      [ "state", "group___model_time_cl.html#afb125fdf1a7251e98daadfb05db9ef8a", null ]
    ] ],
    [ "mmdlTimeClStatusEvent_t", "group___model_time_cl.html#structmmdl_time_cl_status_event__t", [
      [ "elementId", "group___model_time_cl.html#a834a02cfc29910d3ba3456f94f8e5ecc", null ],
      [ "hdr", "group___model_time_cl.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "serverAddr", "group___model_time_cl.html#ad76939bb77602643e575d6a33803e03f", null ],
      [ "state", "group___model_time_cl.html#a82716c2da19cceb7e3ece79fadc23de8", null ]
    ] ],
    [ "mmdlTimeClZoneStatusEvent_t", "group___model_time_cl.html#structmmdl_time_cl_zone_status_event__t", [
      [ "elementId", "group___model_time_cl.html#a834a02cfc29910d3ba3456f94f8e5ecc", null ],
      [ "hdr", "group___model_time_cl.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "offsetCurrent", "group___model_time_cl.html#a5c34f12b7a20c00785ce5823e14a4c87", null ],
      [ "offsetNew", "group___model_time_cl.html#a09d28ff72acd6343e0820c0a332ff729", null ],
      [ "serverAddr", "group___model_time_cl.html#ad76939bb77602643e575d6a33803e03f", null ],
      [ "taiZoneChange", "group___model_time_cl.html#a23b85ab26e2dbc16551527e32c7aa644", null ]
    ] ],
    [ "mmdlTimeClDeltaStatusEvent_t", "group___model_time_cl.html#structmmdl_time_cl_delta_status_event__t", [
      [ "deltaChange", "group___model_time_cl.html#a82922660b262c8fdc39a2164be9671cb", null ],
      [ "deltaCurrent", "group___model_time_cl.html#a23f321fb8e0d482c71b3cb4307f285f2", null ],
      [ "deltaNew", "group___model_time_cl.html#a64916f62c15f0eee76175e1e95e855e8", null ],
      [ "elementId", "group___model_time_cl.html#a834a02cfc29910d3ba3456f94f8e5ecc", null ],
      [ "hdr", "group___model_time_cl.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "serverAddr", "group___model_time_cl.html#ad76939bb77602643e575d6a33803e03f", null ]
    ] ],
    [ "mmdlTimeClRoleStatusEvent_t", "group___model_time_cl.html#structmmdl_time_cl_role_status_event__t", [
      [ "elementId", "group___model_time_cl.html#a834a02cfc29910d3ba3456f94f8e5ecc", null ],
      [ "hdr", "group___model_time_cl.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "serverAddr", "group___model_time_cl.html#ad76939bb77602643e575d6a33803e03f", null ],
      [ "timeRole", "group___model_time_cl.html#a1408ede67be03a1b7933dfa25f568b6a", null ]
    ] ],
    [ "mmdlTimeClEvent_t", "group___model_time_cl.html#unionmmdl_time_cl_event__t", [
      [ "deltaStatusEvent", "group___model_time_cl.html#ad1559f65641d1527d7ddfb43e66b0461", null ],
      [ "hdr", "group___model_time_cl.html#a30db164981dd6b8dc1c5b1ce430bdfe5", null ],
      [ "roleStatusEvent", "group___model_time_cl.html#ad3370f54913aa77dbd41bccbf77f5e7b", null ],
      [ "statusEvent", "group___model_time_cl.html#ad85523b3d35898a9ad6280feaa260c13", null ],
      [ "zoneStatusEvent", "group___model_time_cl.html#a9ce40f02b7dfcadb2d583b9716b44f66", null ]
    ] ],
    [ "MmdlTimeClDeltaGet", "group___model_time_cl.html#ga3c406ee150e3294649613a73f6e822c3", null ],
    [ "MmdlTimeClDeltaSet", "group___model_time_cl.html#gae90d4cafe5e50c7e525a13442d0a43cf", null ],
    [ "MmdlTimeClGet", "group___model_time_cl.html#gab3dbf7d77c42fe641ffeac6b0abde4ef", null ],
    [ "MmdlTimeClHandler", "group___model_time_cl.html#gaf1872791a177f105700f536e8dc4c9d6", null ],
    [ "MmdlTimeClHandlerInit", "group___model_time_cl.html#ga0acea25226bb8c1cb9851553e302d716", null ],
    [ "MmdlTimeClRegister", "group___model_time_cl.html#ga3b7dd18ecc9d12314cddae8208d19446", null ],
    [ "MmdlTimeClRoleGet", "group___model_time_cl.html#ga8d539e0c2b4fd0bda87aa8bd3ec4e93d", null ],
    [ "MmdlTimeClRoleSet", "group___model_time_cl.html#gaae707c8a2254601c578eceb18496cb97", null ],
    [ "MmdlTimeClSet", "group___model_time_cl.html#gadbbc5cbc4e5ed2041fea0250906aaa75", null ],
    [ "MmdlTimeClZoneGet", "group___model_time_cl.html#ga67de489e3e6377173ed2828b133fba1e", null ],
    [ "MmdlTimeClZoneSet", "group___model_time_cl.html#gafbb73f014db5965feadb54895ed5ede6", null ],
    [ "mmdlTimeClHandlerId", "group___model_time_cl.html#ga294c49c6b683deb063919928c186cda0", null ],
    [ "mmdlTimeClRcvdOpcodes", "group___model_time_cl.html#gac11ff13be0238190e69a36c74531ceda", null ]
];