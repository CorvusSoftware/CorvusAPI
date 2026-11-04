// CorvusAPI - CorvusSoftware
// Copyright (C) 2026 CorvusSoftware
// SPDX-License-Identifier: MIT
#include <corvusapi.h>
#include <stdio.h>
#include <string.h>
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x); return 1; } } while (0)
static unsigned callbacks;
static uint32_t pixels[2];
static void video(void *user,const uint32_t *data,uint32_t width,uint32_t height,size_t stride) {
    (void)user;(void)height;(void)stride;++callbacks;
    pixels[0]=data[0];pixels[1]=width==2?data[1]:0;
}
int main(void) {
    struct { corvus_cheats_api api; uint64_t tail; } probe;
    memset(&probe,0xA5,sizeof(probe));
    probe.api.struct_size=sizeof(probe.api);
    corvus_cheats_api before=probe.api;
    REQUIRE(!corvus_get_cheats_api(2,&probe.api) && !memcmp(&before,&probe.api,sizeof(before)));
    probe.api.struct_size=sizeof(probe.api)-1;before=probe.api;
    REQUIRE(!corvus_get_cheats_api(1,&probe.api) && !memcmp(&before,&probe.api,sizeof(before)));
    REQUIRE(!corvus_get_cheats_api(1,NULL));
    probe.api.struct_size=sizeof(probe);uint64_t tail=probe.tail;
    REQUIRE(corvus_get_cheats_api(1,&probe.api) && probe.tail==tail);
    corvus_cheats_api *cheats=&probe.api;
    REQUIRE(cheats->struct_size==sizeof(*cheats) && cheats->api_version==1);
    REQUIRE(cheats->capabilities && cheats->validate && cheats->replace);
    corvus_core_api base={.struct_size=sizeof(base)};
    corvus_subsystem_api subs={.struct_size=sizeof(subs)};
    REQUIRE(corvus_get_api(0,&base) && corvus_get_subsystem_api(1,&subs));
    corvus_host host={.struct_size=sizeof(host),.video_frame=video};
    void *core=base.create(&host);REQUIRE(core);
    REQUIRE(!cheats->capabilities(NULL,0) && !cheats->capabilities(core,0));
    REQUIRE(!cheats->validate(NULL,0,"BYTE=2A") && !cheats->validate(core,0,"BYTE=2A"));
    REQUIRE(!cheats->replace(NULL,0,NULL,0) && !cheats->replace(core,0,NULL,0));
    uint8_t bytes[]={0x11,0x22};
    corvus_game games[2]={{.struct_size=sizeof(corvus_game),.rom_data=bytes,.rom_size=1},
                         {.struct_size=sizeof(corvus_game),.rom_data=bytes+1,.rom_size=1}};
    REQUIRE(base.load_game(core,games));
    REQUIRE(cheats->capabilities(core,0)==CORVUS_CHEAT_FORMAT_CORE_TEXT);
    REQUIRE(!cheats->capabilities(core,1) && !cheats->capabilities(core,UINT32_MAX));
    REQUIRE(!cheats->replace(core,1,NULL,0));
    REQUIRE(!cheats->validate(core,0,NULL) && !cheats->validate(core,0,""));
    REQUIRE(!cheats->validate(core,0,"BYTE=ZZ") && base.last_error(core));
    char code[]="BYTE=2A";const char *active[]={code};
    unsigned old=callbacks;
    REQUIRE(cheats->replace(core,0,active,1) && callbacks==old);
    memset(code,'X',sizeof(code)-1);
    REQUIRE(base.run_frame(core) && pixels[0]==UINT32_C(0xFF2A2A2A));
    old=callbacks;
    REQUIRE(cheats->validate(core,0,"BYTE=55") && callbacks==old);
    REQUIRE(base.run_frame(core) && pixels[0]==UINT32_C(0xFF2A2A2A));
    const char *invalid[]={"BYTE=55","not valid"};old=callbacks;
    REQUIRE(!cheats->replace(core,0,invalid,2) && callbacks==old && base.last_error(core));
    REQUIRE(base.run_frame(core) && pixels[0]==UINT32_C(0xFF2A2A2A));
    REQUIRE(!cheats->replace(core,0,NULL,1) && !cheats->replace(core,0,invalid,129));
    const char *null_code[]={NULL};REQUIRE(!cheats->replace(core,0,null_code,1));
    char huge[CORVUS_CHEATS_MAX_CODE_BYTES+2];memset(huge,'A',sizeof(huge)-1);huge[sizeof(huge)-1]=0;
    REQUIRE(!cheats->validate(core,0,huge));
    const char *maximum[CORVUS_CHEATS_MAX_CODES];
    for(size_t i=0;i<CORVUS_CHEATS_MAX_CODES;++i)maximum[i]="BYTE=2A";
    REQUIRE(cheats->replace(core,0,maximum,CORVUS_CHEATS_MAX_CODES));
    REQUIRE(base.reset(core) && base.run_frame(core) && pixels[0]==UINT32_C(0xFF2A2A2A));
    REQUIRE(cheats->replace(core,0,NULL,0));
    base.unload_game(core);REQUIRE(!cheats->capabilities(core,0));
    REQUIRE(base.load_game(core,games) && base.run_frame(core) && pixels[0]==UINT32_C(0xFF111111));
    base.unload_game(core);
    REQUIRE(subs.load(core,"pair",games,2));
    const char *first[]={"BYTE=33"},*second[]={"BYTE=7F"};
    REQUIRE(cheats->replace(core,1,second,1));
    REQUIRE(base.run_frame(core) && pixels[0]==UINT32_C(0xFF111111) && pixels[1]==UINT32_C(0xFF7F7F7F));
    REQUIRE(cheats->replace(core,0,first,1));
    REQUIRE(base.run_frame(core) && pixels[0]==UINT32_C(0xFF333333) && pixels[1]==UINT32_C(0xFF7F7F7F));
    REQUIRE(!cheats->replace(core,1,invalid,2));
    REQUIRE(base.reset(core) && base.run_frame(core) && pixels[1]==UINT32_C(0xFF7F7F7F));
    base.unload_game(core);REQUIRE(subs.load(core,"pair",games,2) && base.run_frame(core));
    REQUIRE(pixels[0]==UINT32_C(0xFF111111) && pixels[1]==UINT32_C(0xFF222222));
    base.destroy(core);puts("PASS optional native cheats negotiation, atomic lists and slots");return 0;
}
