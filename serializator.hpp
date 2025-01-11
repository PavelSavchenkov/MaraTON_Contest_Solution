#pragma once

#include <random>

#include "utils.h"
#include "compressor.hpp"
#include "huffman.hpp"
#include "lz_compressor_bits.hpp"

namespace Serializator {
const std::string compressed_dict =
        "IAAAAICAAAAA////gUAAv8QQAAf//VK4AH//////4oQAB8N8ACAEAAGAMAAP8BFAAH/oggAD/aSHAA/wEjwAP/RK8ADog0AD/QRvAASCnAA/+EpAAP//UjkAA/+lSwAP//UoCAA//+qRyAB/oLwAAP//uj9AAf//BDHAA/7klwAGAKcAD/kSDAAf/+VG9AA//9SMqAB/tIhwAP//woLAAfFTAAP//gkWAAf6iSYAD////7MaAB//7U+EAD/gQhwAP8BLzAA//96T/AA//+SDPAA////K0UAAP/RH1AA////u4BwAP+BFeAA/////6wJcAD/+VzgAP+BhnAA/1EaYAD//+qHkAD4B3AA////Ay0wAP/5QJAA////kyFAAP////9sU3AA//lAkAD//0IjQAD/uXYwAP//kgjwAP//ajAgAP95V2AA//8CBHAA/5GsUAD///9rHkAA/3lOcAB//8EeqAAuCDAAH//gTQgAD/0RxIAH//vShUAD/////bGTIAH///6mskAA//9qdygAf/+9NMAAP///wN7YAB//70yZAA//8SBXgAf////5YK7AA/3kJcAB/////wgKcAD///oPaAB/wKU8AD//7oXWAB////hkQQAP///3t3MAB//6EEXAA//96TfgAf///yYI8AD///8raMgAf/zQFAA///CNWgAf////Zc8AD///p7qAB//8EJUAA/7lD0AB///+p22AAP///+tNgAB/nJ3QAD////TEaAAf///lYGcAD///87qVgAf///iZn0AD///+7A7gAf///tc2YAD//xIG+AB///+1toQAP/////8QpAAf//BJ+AAP//estgAH//vSBKAB/////ghFSAB/////niiaAB////drPIAH////+KFdwAH///72ZpgAf///9bN4AB//8kRtAAf///3ZROAB/9IEeAB//6ELngAf5yo/AAf////4owDgAf////5YhKgAf///3bOIAB//IR4AB///+hg3oAH/8k5AAH///6GOmAAf////cFAAB///05/AAf+CIIgAf/+dNLYAH////++LqIAH//wTY0AB////9nHAAH/cjEoAH///4mWGgAf7SCPAAf///iYSYAB/////vgIsAB///+VtZYAH//vRxQAB//8EIsAAf/yFdgAf//BIhYAH/cvZYAH//oQ0YAB8MsoAH//wQVWAB//50KzAAf/+JDLgAH///7Wf2AAf/+9M3gAH///+HSsAAf//dQPAAH/8wBIAH///4GcjAAf///vZEgAB//90mzAAf/+pRCoAH///9WNpgAf7yBDgAf////6piXgAf/+1GRIAH//////gr3mAB///+dqvYAH//wVpUAB//70cbgAf//pX8oAH///7W1VAAf///vY5YAB///+hhdgAH/80HYAH///728iAAf///6d8cAB////V23IAH/cjQwAH///6mqJgAf/SNDAAf/+9DwgAH//6VW8AB///+1yKQAH/IgV4AH//3RoCAB//7UCDAAf///6bNGAB///+1lUwAH//nS0QAB/4IVmAB//71nfAAf6CU/AAf///3ZuAAB//6kcAAAf///le7+AB///+d2SYAH/IiqwAH////XCPAAf7yDCgAf//JX5oAH//tTEIAB//70WSAAP///0MLqgAf//9CXYAD///87FpYAH///6G9BQAP/5KEgAH//6UAlAA////60TmAB////dkpcAD//zotjgAf//pOFUAD///9rRQQAH//nQIcAA////U00YAB///UCJAAP95AhwAH///+mlrQAP///6tJbAAf9yCLAAP///xM8sgAf//VCzsAD//////z1E5AAf///iYKEAA//////89AowAH///7WLmwAP////88KOYAH///529rAAP////+UbaIAH//9RVCAA////e5BUAB////JjCsAD///8rXO4AH///72BHgAP////+ULo4AH//9R5jAA///6MLoAH///8mFzwAP//ah1uAB/////nhSUAA//86eMAAH////+qOMUAD//6oJcAAf///naYtAA/////xRWeAAf///1c15AA/zmkXgAf/////+evfUAD////TVGwAH////+WMFsAD//////z0GFgAf///3ZKBAA////u3tKAB//4lKrgAP//Kgz6AB//+kPfwAP///7sfjAAf///tbPlAA//86KogAH///8mJ3wAP//qpn0AB/////vgebAA//////9VS24AH//////lqPKAA//mMMAAf///6YGOAA////////LgRKAB///+9mzsAD/////PAW4AB///+phj8AD////7IogAH///7WEIgAP///xMU4AAf///lYJ0AA//86IsgAH////+qDyAAD//6om0gAf/+hG+sAD//7oaxgAf///ian5AA//+qvKwAH//vSSLAA/////1ShfgAf/+1LXMAD///+7XUIAH///8GBNgAP//elJWAB/////nhr9AA////gwKcAB/vKSUAA//+STl4AH//9TZUAA////+yRGAB///+9mJAAD//9JKlgAf//VJOMAD//7pUsgAf///6eW3AA////66LaAB//70q8AAP//kjh0AB////1nLEAD/eXOsAB//91QngAP////+UupAAH//tVtsAA//////99AlwAH/c3e8AD/wQJcAB//50w3QAP///5PTZAAf/////+epEwAD/////LE7uAB//91d4AAP//0jCgAB///+ViFEAD//zoNaAAf///iZleAA////+5f6AB/////likUAA//+6HsoAH/gj+wAD//zrFPgAf//JEcEAD///97EIIAH///7W6DAAP//+grcAB///+BsC8AD///+7Et4AH//3UI/AA//8SwlQAH//tQW1AA////u3hgAB//////57s5AAP///xMBngAf////5YaYgAP//Oj7GAB///+Jgh0AD//7oFtgAf//VIacAD///9DJN4AH//vRhVAA////e+aiAB///+pgdUAD////rT/gAH+86EUAD///87M5QAH////+KNvMAD///9rCe4AH///526zAAP//+if4AB//6lUpQAP///2tNDAAf/+9NvMAD//8JDegAf9yY2wAP///0MYhAAf/////+etOYAD///qtFAAf///yapLAAf////Y3nAAf/+1OGsAB///0jooAH//////nrBEgAf//JIpSAB///+VldqAB////9jbOAB/wINCgAf///nbTogAf//9FMAAB//////4qUcIAH//////////wCi6AAf///////////////5gAEAAf/SMLAAH///52Yp4AH//6QfkAAf//1B4CAB/nIReAAf//hArkAB////dwWCAB/////////////////////xAIpgAf///yaV7AAf///9Zc6gAf6iEXAAH///SgSAAf///tYMIAAf///3bhCAAf///oaurgAf//9I+4AB///0pywAH///72dGQAH////+KOVwAB///UMFgAH///92EPgAH////2UroAH//oQRaAAf///naAMAAf9yQcgAH///8Gyl4AH///52lM4AH+gyX8AB////djAIAB//7032gAH//6Q4+AAf///vbFKAAf//VLEKAB//50RRIAH//wQGJAAf//9K/cAB/wIIMAAf/+dCveAB////dupEAB//+EacAAH/chMsAB///+dlPGAB////dj+6AB//8E2r4AH///53AXwAH//ySIzgAf/+VBBQAB/////3kUnAAf9zJL4AH///52XmIAH///8nEdAAH////Wg9QAH+Ig2WAB//9UIPIAH////+eA5UAB///+9vHUAB//6kGg4AH///7XFToAH/8rjqAB//8kA2oAH//nQQ7gAf////5ZGDQAH///92FVYAH/////eOycAB///+9wCqAB//+lC8gAH///7WXtAAH///93IyAAH/8nxqAB////BhXmAB///+llIGAB/9J9eAAf//pDWSAB/////lixhAAf9yN34AH///53BSAAH///6GeOAAH+8mMYAB//+ECIAAH//vVJxAAf/+dCZWAB////JkmkAB////VrvSAB//KEugAf//dC1YAB/wIFZAAf+iAyoAH////WhGoAH+8h7UAB////Jo74AB////1gD+AB////9q+kAB//50i9wAH//vQRfgAf+CPToAH/////eN0kAB//NX+gAf//pBJsAB//7UEuoAH//vSZZAAf///iYh3gAf9SAK4AH//3Rx+gAf///gaXtgAf/+dQDEAB/4IsCgAf///nYdtgAf9yG1oAH/gi3GAB//70JJgAH///73HCIAH//oQWggAf9y+HAAH///6G3EQAH//vQP0AAf//dTkCAB///+9l5qAB///+hurIAB//IA/gAf////bKkAAf+CBUYAH//3SnSAAf+iGBAAH//tRhNAAf/+dIcAAB////JlKwAB////9iDOAB///+1mnUAB/3IgEAAf//BAdCAB//JyBgAf///qZFeAAf//VGd4AB///+9yMSAB//4kAT4AH///92xlQAH//1Ru+gAf/ykF4AH//////qrBnAAf///tYOhAAf7yfNIAH/g3WQAB/////3jspAAf7yEa4AH////2o0wAH////+KKxcAB////1pUGAB//70kuoAH//gQYYgAf/+dCz4AB/6IN5gAf////c+MgAf/+1GNQAB//70M9oAH///8mfe4AH//9QTugAf/yHyQAH///4nAxoAH////+WNUIAB//70fKAAH/83uGAB//8EHyQAH//nQW9AAf+CHOgAH//3TEEAAf///vY/VAAf///oYzHAAf//9AtKAB//91b6oAH//gQ6TgAf/+9Lb2AB////d39OAB//70LhYAH//oQiDgAf///vcK2gAf////75fy4AH//iQrvAAf////c8oAAf///tZqwAAf/+9KTmAB////h1/kAB//70r7IAH//nQPkgAf///vYMeAAf+CXPoAH///R1DAAf/////++jWoAB/////vjMbgAf/+dB7UAB//8EDoQAH///53XuQAH///92l2wAH/////+CmgAB///+dn3MAB////dyQKAB//6kUdAAH///7WrLYAH/84RyAB//6Ej1IAH///53KCIAH///93JhYAH//yRTPgAf////95oMoAH//tQBTgAf///3cbigAf//////eqlsAB/3JVqgAf////4oCa4AH////XYB4AH+8phyAB/////vk92gAf///gYv9AAf///vdIigAf///nbVvAAf///iZSggAf/+1ADOAB////d0yyAB//90vgYAH///4mja4AH//tRWegAf////d9vAAf///iYAQAAf/+dFZKAB///+15soAB///0fJAAH//wQAzgAf////aRGAAf/////5Kq4AH//3TakgAf///vY6AgAf//hH7MAB//////57JlQAH///52ZyoAH///TYtgAf//pFKkAB//70wxwAH///5WGw4AH//iQFZgAf////dIhgAf/+1B0KAB////d7fEAB//6EfCIAH////+eVZ8AB/vPEegAf////5438gAH///+m2QQAH////XXJIAH////2ybAAH//gQ2LAAf/+dAqsAB/////3lU0AAf7zyTAAH///4GR2oAH///53dYYAH/8lCYAB///+hoJWAB//I87gAf///nYSLgAf///yYyogAf////fDngAf//1PVgAB//90/44AH/////+LSmAB///+9p3SAB//8EFjYAH/cjLSAB/////vi6HAAf///3bpFgAf///oYeKAAf//9Bu8AB///1XxwAH///6mAT4AH///92k0YAH//tQb/AAf/+JCbIAB///UVQAAH//nRYSgAf///iZm7gAf///ndXWgAf////5ZSOIAH//nWHNAAf//JDJSAB///+1lDYAB//90toIAH//3Qv1gAf//hLNIAB//50oOwAH///53kMIAH//wQy8AAf///3bnpgAf/+9JuWAB////J8lSAB///+1lpGAB///+du+4AB///0qVwAH///52a9QAH////+CShiAB///0JdgAH/AkAoAB///+dm0iAB/vKLSAAf///wYA7gAf///vcJOgAf/+dMuaAB//+l1rgAH//lQ9bAAf///nbJXgAf//JDT8AB/9IPggAf///nYt6AAf/+pEKCAB////1lxCAB//70B/AAH///53bm4AH///6m+CYAH//1SHUAAf///vZtUgAf///idMpgAf///9b8fgAf////95swIAH///729boAH////++GSYAA////uzvIAAf//JGqsAA////q24+AAf//9DFMAA//+CKa0AB//70DGEAD//3oUgYAH///+mF+sAD////ra2MAB/vJCWwAP//kjBQAAf///////nyAiwAP///////25V+IAH//vRz3AAP//////xXKTAAf//////+nemAA/////7x/lQAH//3T2cgAP///ztHnYAH///6mAH8AD/////bGOaAAf////Zp8QAP/5DDaAB///+9ugRAA////kxHvgAf///tahCwAP//ehZHgAf/////+evpDAA//+6FgwAB////piyXAA///qKMyAB//70zeAAD////DFX0AB///+dqaRAA////e1XEAAf//BA7vAA//ksTgAH///92ShgAD//1ILiAAH///9XD1IAD///ppkwAH///52ZD0AD///8TaZsAB///+1gOhAA///6fB6AB/////3kEiAAP///zuHdoAH//vQVWwAP/BGAgAB//50eK0AD//0IaTAAH////+ePmiAA////Ox39AAf/+JGlrAA///qHiiAB//50pYIAD//0IypYAH///TQAwAP//+m32gAf///ybDpwAP///3tqeQAH///5WAIcAD///////0u1QAH///52GzAAD//1IFVgAH///7WmGEAD////7d42AB///0TucAD///87EuoAB//8EBeUAD///97IYKAB//8kL2oAD///oRIYAH///9Wx3gAD//7oOWoAH///6nEEoAD/////bISXAAf///vYhVQAP//+hgIgAf//BC58AA////u0IYAAf//9A8cAA//////xqHQAH//gQwWQAP//uiM/gAf//dDPKAA////A01ogAf//9GB2AA///6QnOAB///+prj3AA//+6AhQAB/1IAdwAP///xM8/gAH///9WkCkAD///oYn4AH//3RsTwAP////9UWG6AB///+1kJ2AA/3lPzAAH///72iKgAD///+TJMCAB//5UfMcAD///+7GnwAB///+9w5EAA////0w7EgAf///1YCmwAP//+gEaAAf/////++vgSAA/////////////9pRbQAH///9WlTIAD///97KgAAB/qJkMgAP//qh74gAf/+dBQVAA/////5SM7QAH////////9GnEAD/////////////////9VNmgAf////74BJMAD/0VNSgAf/+VC+AAA///////9AP6AB/////////////////////////yavtAAP///zshAQAH/ciCeAA/////wQnUQAH///52jnAAD//zpdPoAH///6miqAAD/////PBXUgAf//////WyNgAA//////99BCsAB///0wggAD////TSSkAB////1sIIAA////O24FAAf////////7/HpEAD//3ohJgAH///4GTrQAD/+WTpgAf///3cJFgAP///1Nz8AAH///5WySQAD//3obgwAH///73My0AD////TO9uAB//9UhVUAD/////fHkfAAf/////++vpxAA/////3wlWgAH/civ7AA/////wQBaYAH///52IHgAD//7oCxwAH///4mBiUAD///+rOvmAB///0rQgAD///opHwAH////+eSQuAA////U4s3gAf/+VDwFAA///6G4cAB///+hmgNAA////ezxbAAf/+dHGjAA////01zwAAf////74AI8AD///9rnKGAB///+9t5WAA////u46BgAf///ibaVQAP///7ud8wAH//9SetQAP////sBqQAH///72f+kAD///+TIfeAB///+1hTMAA////Oz1NgAf//9SFbAA////0xp1gAf///9Z1NQAP//ugvLgAf/+9KgrAA///ConKAB//70yNAAD/+SGQAAf//hGViAA/////zxf4oAH/////+L/lAA//////89pKqAB///+9lxMAA//+SAb0AB///U4w0AD/uURUgAf///oYrpAAP//+lRBgAf///3ZmYQAP//uos/AAf+Crp8AD///97JsaAB///0Ns8AD/////BHsBgAf////bc6AAP//////PRECAAf///vZPegAP///xM2RwAH////+eOcuAA///qm4IAB///0CSgAD///8TDImAB////9x06AA////63GKgAf/+9TxjAA//+6hrKAB///+JmMwAA////KxJjgAf///3b83wAP//+nkRAAf///gYQcgAP///3sazQAH////2888AD///97LFyAB//+FANgAD///+7JBAAB/3IEzwAP///zuZNgAH//oQNIAAP//uhwegAf//9Bm5AA////0xIjAAf////5Y8VMAD//zoPWAAH//3RXYwAP///4MVzQAH///92uzAAD//7pk3gAH///72aJ4AD//4IVzIAH///53CcAAD///87P5GAB////9qDcAA/////wRyaIAH///92aJ4AD///qv+gAH///+mEcAAD////7RzuAB///+1tFmAA////u5a6AAf/yRewAD/////VFrGgAf///tZb2wAP//Oi0vAAf////94e2sAD///87mraAB////hkpMAA/////7wYi4AH+8rMqAA////02oFAAf///1bRvgAP//ui8rgAf/+9W9mAA////w1xEAAf///5YAdwAP////8sBCyAB//901g4AD///87Q1MAB//8k/pMAD/6VU5gAf///vcuwgAP///8MMSYAH/////+VR/AA////OyWNgAf//9Nz5AA////ezZCgAf///yaRSgAP///2sCd4AH/////eJRVAA/7k54QAH/////eKpEAA////EwsUAAf///vaUdAAP///+sTRQAH///52OVAAD/////PD5RAAf///idivgAP///6tTkQAH/8ix/AA/////7yuVAAH//3R2XAAP////+UPBkAB////VgNCAA//86hJQAB//90r8IAD/////FFK2gAf/+1QdDAA//+6b9KAB////ds/UAA////gyNYgAf/+9ObHAA//////yQRwAH//3Sv6gAP//0o1MAAf////9ZY0QAD///87FIKAB///+dg2fAA//////99ZMiAB///////////////////////+RE9dAA//+qCuYAB//91CeoAD///////8WlxoAB////////fS7jAA////64IigAf//pLNRAA////61IWgAf///nZI2QAP////+8pGQAB///03wUAD///97Ht6AB//6ECVwAD/+WuWgAf///qbargAP///7sAO4AH///5WelkAD////7p44AB/6J9uwAP///6twFYAH///TZQQAP///ztYhoAH///8mDE8AD///9rML6AB//90YY0AD/0WMTAAf/////YV6cAD///oefwAH///72UGMAD///o0YQAH//////////////rZmRgAP//////////sDFZgAf//////////////////////////SRBqAA////////rgoYAAf/+dH0NAA////0zt0AAf////aBZQAP///ytRrAAH//nVhCwAP///3tMhYAH///8mW9YAD/////LE2ngAf7yD2wAD/////BJ0yAAf////ZlrgAP+5RO6AB////ducYAA/////1TBoAAH///7WqL4AD///ophwAH////3YaoAD/kQd6gAf///9ZwYgAP//OoZ7AAf////74NxEAD///9TR+UAB////VobbAA/////3wZ14AH///RfQAAP//UhOzAAf///9c7sQAP+5grqAB///+96HnAA////E1epAAf////5Zb1kAD/OVCmAAf////95MtEAD///+DC5MAB/////hixdgAP///2t9LYAH//vRvbAAP//////fbqEAAf////////9+ckAAD/////////RxHxAAf///vYFoAAP//kiffAAf///vbwAgAP//6sCGAAf////94NowAD/////vBZfAAf////d+YQAP//eiELgAf///6Zq3QAP///+sgNQAH///RbYQAP//umwIAAf////55G90AD///97eCEAB///+Bxk6AA//961FOAB///+9y8hAA/////zwrlAAH///8mcm8AD///97cjCAB///UNM8AD///pGKYAH/////qWaqAA////uycggAf//////2xIBAA///6blQAB//////////////6GDiEAD////7FlUAB//////////8AAMcAD/////////////////////yBLUAAf///6cKxgAP/phe4AB//////+qA34AD///////++AGuAB//////////////////5sKa0AD//////////////////////////yQDagAH//////////////////////////////v4sPAAP//+m17AAf///vdR7QAP///7ucGgAH///4G6G0AD/////PKu1gAf////95DIkAD//9IPygAH////+eLCMAA////q3aGgAf//dHLxAA//96alWAB//////6qlZgAD/////LGqxAAf///vYLrAAP//////fSG+AAf//JE2GAA////+yrLgAf/////Y6XUAD///+7D/EAB/////3lObQAP//whEbAAf//dIIXAA/////////////wKkIoAH/////////////////////////l0rUAA///CzKgAB/////vgpdwAP/RDCwAB////V2SyAA//mKMQAH///XJPQAP////8UXf8AB///+V8nMAA////+wUxgAf///3aI0AAP//////////////////////kSkZgAf//9ZStAA////ezFIgAf///qYu8AAP/////sAN6AB////96gTAA//86HVKAB//6EFVQAD///+7dJwAB/////vh76wAP////vtc4AH///53kEQAD//////EJxgAf///oZi6QAP//ulfDgAf////55ZB8AD///prfAAH///6nAokAD////71xQAB///+1qKlAA//96eOwAB/////vl+1wAP///1PeHwAH///5WRjgAD/////vFf1gAf///ncJjAAP////////////////////////9LV3oAB///////////////////////+pAJnAA////e2xaAAf////bdNAAP///3t+sQAH//////////////////////6K5bAAP////88GfAAB///1XYYAD///87RCQAB/////3gYmgAP////88ttQAB////p8PDAA/////////2/pJYAH//////////////////////pI26wAP////+UHh+AB/9IVkAAP///zsOrwAH///6GF20AD//3qRsgAH////+eXo+AA/////zxpEIAH///////6sutoAD////roAcAB/3MRbgAP//////1QZhAAf/////5osoAD///+rgrMAB///0k4oAD///////////8BFPsAB///+p5hzAA//////9tHG2AB///+dwRyAA////E3efgAf///////////////////////5VyLAAP/////s2rAAB//////////////////////4SAo8AD///+7PdvAA///////Vc6fAA/////////////////////////7sbLEAD/////vAxoAAP///////////////////w8AmcAD///o5OEAD////////+GrfAA////e3aeQAP///xNnz4AD/////LD3AAAP95HaiAA////OzN8QAP////9UXrAAA////a0QPAAP////////4OV8AD/////////////////////////////////////////FwoEgAP/5YLoAA////////Pny3QAP///zsuJ4AD/////////////GgVdwAP///+sESkAD/eUOzwAP////////8XAG2AA/////////78APcAD//////////////////zYFe0AD///8TQoiAA/////////jc+wAP///////////////////////////////////////0YV7oAD/uYaeQAP////98YlRAA////kzlQwAP///2tHdcAD///owPsAD///97fSVAA////UxAAQAP////+sIZZAA/7lZ3wAD///////////////////////9iIx+AA/////zxFacAD////rRDYAA/////7whzsAD///oiOAAD///////8WAQ8AA/////////////////////////3sEFEAD/////////////////////////SwicQAP////98IwSAA//kGhkAD////TCOHAA//8qcUoAA//////99O3WAA////0wg1AAP////88ACHAA////////bgTTQAP95TWgAA////////Vg3BgAP////////////////////////////////////////////8KG5FAA/////3wE/AAD//////////////////////5EXHYAD//////////////////////////0wAo8AD///87gnyAA////////vj7JgAP/5IOQAA////k2XpQAP///////253oUAD/////PACvAAP///7sZMcAD//3qO1sAD///////////+BTcOAA//////////////////////////9kIS2AA////+0rdgAP////////////////////////97ERwAA/7kwecAD/////////fzaOwAP//uholQAP///ztX/IAD//////wUAb0AD/////PC6OAAP//////////QAV5gAP///3t7oQAD//////////////////////////4wCfcAD///9TOtJAA///////////////////////////8LS0AA/////////////////////8gJaoAD///+7dXvAA////////////WQ0ZAAP//aiKgAAP//+jSVwAP////////8/jsLAA////QytXgAP/////8VKbAA/////3xz2UAD//////////////////////4kJXsAD//wIgb0AD///87QWOAA////e2AuAAP////88ZnYAA//////weIIAD/////vGklAAP///xNrAUAD/////LHCGgAP////+8a+mAA///6WgHAA////uzqSAAP//khRlwAP//6hm2wAP////+8iRNAA//lqQQAD/////PC41QAP///4MAH8AD///+7U39AA////O0NkAAP///3txT0AD///+DDzGAA/////3yP4YAD//zpeKAAD/////PBl0gAP//////VRNVwAP///6sAvsAD///87YYgAA//////8VAC3AA///qiNBAA//////xzqUAD//////wUAZYAD////LXIXAA///qkC1AA////wwBdgAP//ul3HgAP//////FQAzwAP//qj6FwAP/////EAD3AA//////89ADHAA////O0UGQAP//////FQBrgAP///6tC/gAD///87dgPAA/////zyaDkAD///+TWsgAA////a1L7wAP//OgF4AAP//////xQEEgAP///////////wEATcAD/////PDgFQAP//um47wAP///9MS+oAD////rAB/AA////e4KVgAP////88VlRAA//////89AlzAA///////FAThAA////OzP5AAP//////BQMMAAP///3tU14AD//////1UDT8AD///////8uAEPAA//kbDIAD/////////PwCxQAP////8UMamAA/////6wAJcAD///o+hAAD/////////////2gAKAAP///6syOQAD////7NyyAA//96WHYAA////uyMQgAP//////fSdvQAP////98jPoAA////UyFYQAP///////////////////////////////////////////////////////////////////////////////////////////////0gACgAD///+7c7+AA/+kjMMAD//////////wCLEMAD///+7P/FAA////w6HFAAP////98oaFAA/////5RL2MAD/////rBsPAAP////98nebAA////eyW1QAP///1N5msAD/////bABFgAP//+kdxAAP/////8dgwAA//////8VFQ1AA///////9FLnAA///qKRWAA//////wKfwAD////7fn3AA////uzi+QAP////uhukAD/////FA+DgAP///6sBfcAD///87PXxAA/////zyE8gAD////TIElAA/////ywfogAD///+7jZkAA////w0ogwAP////88OS/AA//96O7PAA////O1ZUwAP////9UVQsAA//////89A85AA/////ywyP4AD/////vJIXwAP////ttf0AD//////30ArQAD/////VBUFwAP//////bRYqgAP//////vZSYAAP/////8kiQAA/////7wrV4AD//////////////////////8lavYAD//wIEDIAD/+ZYaQAP////98baZAA//+SYnDAA/////yyA14AD///87bDUAA///6Yq5AA////OxP9QAP//Ans8gAP/////8bahAA///6Y7hAA////O2irwAP////8Ei4/AA/////zwGGQAD///pSxoAD//7ptLUAD///////////9ZFFCAA//////////////////////////98F54AA/////3wzywAD/////vF5BQAP///3syXYAD///87YTxAA////UwkjgAP///7uavwAD/////bBZMQAP///7tbNIAD/////fAl2wAP/5VdZAA////AxzWQAP//elPCAAP//+gZ9AAP///0NCCQAD///97SthAA////e3iJgAP//////PaOlQAP///zsWW0AD////TVA2AA////61QrQAP//+pZQgAP////sQvYAD/////vD95QAP////////8/IxBAA//////xMYcAD//9IY6gAD///8rTgTAA//////xuIYAD///oAjwAD///97jUwAA/////7yRhIAD///+7c2GAA//////////////////////////+kISDAA////q4T+wAP//uiYJAAP///////xYFlIAD////////+ADfAA/////////28/cIAD//7qDRwAD/////fApEAAP///7tmioAD///+TAB/AA//////////////////////gzvgAD////////////////////////KIDWAA////////////////////////ogCXwAP/////////////////////4JCKAA////ewRjAAP///1NJVwAD////7AB/AA////q4nnwAP///////74JMcAD//7oKfcAD//////wWOuMAD/+U+rwAP///////////0EJ2QAD//////xVGQMAD/////7HvpgAP//////////////////////iSeVwAP/5FgMAA/////1Q+0MAD///9rCV/AA//86XzYAA///CG+GAA/////zwQt4AD///////////////////////////////////////////////+cRMXAA/7l0xcAD/////vCI/wAP////8UA67AA//////89oS2AA///////////////////////////8AUEAA/////3xtyUAD//////////xh1d4AD///////0UIsAD///+rZhMAA////uy8fgAP////////////////////////9LKHEAA////////////////////////////DTx9gAP//////VTP+AAP////+8oLMAA/////yxx7EAD//////////////8MEkMAD/////////////////////kAjnAAP/5BGEAA////O3s/AAP////8UBKiAA////a5i/gAP////98HMHAA////////BiFAQAP///ztPNoAD//////wUiesAD//7pb04AD///////0iJUAD///////9+ADXAA//////+FHwlAA////+6zugAP//////PR9DAAP///////1YAQ8AD//2pqjYAD//////AB1gAP//////BRnXgAP///zt4uIAD//////1UACgAD///////+uvPLAA/3lvZ8AD///////////+JAAIAA////+xmIgAP////ueWsAD////7r9KAA////0w3GQAP//////PaIAQAP//////LbWmgAP////uUGoAD//////A1FwAP////+UBzUAA/////zyBLsAD/////bIe2gAP/////8GLnAA//86HVbAA/////zyrIUAD////TKLhAA/////2yAiEAD///97a/aAA///////9LMlAA////u7RlQAP///7uSMwAD//////xVJNsAD//////601SoAD///+7CbGAA////e0l2gAP///////////////////58IhEAD///9rkm2AA///6BX0AA//8SGVUAA//+6kzgAA////////LgFMgAP////88GklAA//+SBiAAA/////ywBkgAD//zp0TwAD///pvHgAD/////1Fe+wAP////+sDyZAA//86mm5AA////+4XxgAP///////WKwwAP//////////CGGoAAP////+8NVwAA////Ow2kwAP////+8vM4AA////eywhwAP////88pdeAA////07tiQAP////////+vEQ3AA////////PjHnAAP///////7676cAD//////////////////45jhAAD//////////////////////////4yj9AAD////7A0bAA//////99DvuAA//kpAQAD//////0UBJQAD/////////////////////iBiJwAP//emm5gAP///1NMl4AD/////bApRgAP//OpChQAP/////ULRoAA//////9tN1xAA//+6BDvAA//+6VZ/AA/////xRnaAAD/////PDM9QAP/////sTYwAA///////9O35AA//lkFwAD/////RIBlgAP/5RIhAA////e6IwAAP////////////////////////9jA3wAA//////yyyUAD///8rmZ/AA////OwzCgAP////98sSPAA/////1R424AD///+7m6wAA////////rhHBgAP////98VKlAA//////yvmMAD////7owQAA/////xRfUIAD/////LIGSwAP////98NACAA/////7wFQ0AD///+7AS5AA////e0jlgAP//////BWVWQAP///7sfeEAD/uSnBQAP////+EAaWAA////////////////////////yh4agAP//OlfLAAP////8UrpXAA/////////z9+I8AD/////////////shnsAAP///////////////////////wpOOUAD///////////////////////8SAJnAA////e09ywAP////98MiVAA////////lgduAAP//////PcYdgAP//////rRJQwAP////98NHZAA////+5JyQAP//////////////AyZPAAP////sUFMAD///////++PzVAA//////////////////////////+MAuWAA//////////////////////9hCAxAA////+5HfwAP//////rQDJgAP//ujUsgAP////88rl2AA/////////9cLJEAD//////y0nVIAD//////////wAwv4AD//////z0KU8AD///////04JMAD//9IDcQAD/////PIEiAAP///////y5jSAAD/////PMkXgAP////////5OgIAD///////////+BD5BAA////////////////////////0lpJQAP//epxQwAP///////c94QAP//////////wB3AQAP//////////////W18bQAP////////////////////////+7S5dAA//+6enWAA/////////1cYmYAD///////////+BGOFAA//9qfKBAA/////////////////////////////84hicAD/////fNQbAAP//////PRUpQAP///////xYzO4AD/////vBY1wAP////+sxmeAA////+zYjAAP/5LR3AA/////xQUBMAD/////////PynQgAP////+seKcAA//////////8ACUnAA//////////////////////9JE8LAA////////1jLHAAP///6tC2YAD//zouGYAD//////////wBXR0AD//////////////////////////yQh5sAD/////7AChwAP//////////////////////////zGz0gAP//////////////////////////zC3fwAP////////////////////////+LY2TAA/////////////////////////0sHnIAD//////70XaQAD//3oSBgAD////TrW1AA////6zBEQAP//////PQlRQAP/5dWAAA////03thAAP//////LYE3wAP/////8fKFAA//////89Iz5AA////////BgU6AAP///////74jRkAD//3okaQAD///////9WQpdAA////////LgA5wAP//+lHFgAP////////4Ae4AD//////////8gL2EAD/////PHSuQAP//+mJSAAP/////////XCjNAA////////rgDFQAP//OoIUQAP////////////8aKloAA//////////+wKmIAA/////3yaPcAD//////9UhQgAD/////////rwWewAP////sFQMAD//3qz4sAD/////////B+I9QAP///7sE4sAD///////////////////9PXDbAA////O8nWgAP////98AuuAA/////////0cBC0AD/////////v0HpAAP//esRZAAP////+8QZuAA/////1QpngAD///+rm/7AA/////////gd4gAP///3sm1QAD/////////1zZ7gAP//////LXKLgAP/////82XNAA///ChKNAA////////////////////////////HTeKwAP//////bdOEwAP//+iHywAP///////TeawAP////////////////////////9jFckAA///////////////////////////8PbcAA//////////////////////8JCshAA////e2EFQAP///////////////////98cUsAD//////20db8AD/////////////////BRJZAAP////////4IcYAD///87tXxAA/////////////xoyt4AD//////y0aXkAD/////PKSfwAP//////////2AWSQAP////////8vAFnAA/////3zL0sAD///////8+D2lAA/////5Qsl4AD///////////ABAwAD/////PJaPwAP///////////0FgrEAD//////////////////////////0wPIMAD///////2Mb4AD////////+A2kAA///6V1XAA////0y5mAAP///////S8dAAP///////////////////zccnAAD///+DHpDAA/////7x8hUAD////////////////cJ7DAA//////////+wCfDAA////uy41wAP////////////////8FhLxAA///////9I/5AA////////////////hBv7gAP///3ulocAD///////////8ZMrAAA////////////cQBLwAP///////34OHwAD///////////////////////+iCcDAA////////PhQNAAP///////24scUAD///////////////////////8KZSyAA//865cOAA////////////////////////UgGpAAP///3ufWAAD////////+X3NAA//kxTEAD//////9ViH4AD/////////7wkJAAP//uu2ZAAP//QiigwAP////88FB4AA////////RizpAAP+5tZuAA////////////////3AP9AAP///////y4QW8AD///////////////////////8KDLgAA////////FgIYgAP///////XHoAAP//6mBqwAP/////8WMHAA///////////IE8LAA//96EsWAA////////BgBXwAP//////fQEwwAP///////74CGEAD///////++FHAAA//mCRoAD//////////////////////+EFeYAD//////60aVgAD///////9+LLWAA/////////3835AAD//7qsp0AD//////////////////////////6RKykAD//////////7AEukAD/////////vwD1gAP///////UcdQAP////////8XNtKAA/////+yUEEAD///////////9BDojAA//////////9ABw5AA/////3wwRAAD////////////////cBa1AA//////8tHVQAA/////zy8PQAD///////9GHJsAA//////////////////8WRVDAA////////LjvbAAP///////////8ETfIAD/////PMY1gAP//////////////////////oTabQAP////////////+CDzpAA////////7ga7AAP//////////QCxMQAP//emS4gAP//////////////////////CQWlwAP///////9ZkBgAD/////////PwA5wAP////////9vQj4AA//////99A7VAA/////////0ABZQQ=";

std::basic_string<uint8_t> uncompress_dict() {
    return {};

    td::Slice slice(compressed_dict.data(), compressed_dict.size());
    std::string dict_bytes_str = td::base64_decode(slice).move_as_ok();
    std::basic_string<uint8_t> dict_bytes(reinterpret_cast<uint8_t *>(dict_bytes_str.data()), dict_bytes_str.size());
    const auto final_dict = lz_compressor_bits::decompress(dict_bytes);
    // std::cout << "final_dict.size() = " << final_dict.size() << std::endl;
    return final_dict;
}

const std::basic_string<uint8_t> uncompressed_dict = uncompress_dict();

static const uint8_t BITS_DATA_SIZE = 10;

struct MyCell {
    unsigned cnt_refs{-1u};
    std::array<unsigned, 4> refs{};
    std::array<td::Ref<vm::Cell>, 4> refs_as_cells{};

    bool is_special{};

    std::basic_string<uint8_t> data{};
    unsigned cnt_bits{-1u};

    CellType get_type() const {
        if (!is_special) {
            return CellType::Ordinary;
        }
        const auto type = static_cast<CellType>((data.at(0) >> 6) + 1);
        CHECK(type != CellType::Ordinary);
        return type;
    }

    MyCell() = default;

    explicit MyCell(td::Ref<vm::Cell> cell) {
        vm::Cell::LoadedCell cell_loaded = cell->load_cell().move_as_ok();
        CellType cell_type = cell_loaded.data_cell->special_type();
        is_special = (cell_type != CellType::Ordinary);
        cnt_refs = cell_loaded.data_cell->get_refs_cnt();

        cnt_bits = cell_loaded.data_cell->size();

        vm::CellSlice cell_slice = cell_to_slice(cell);

        std::basic_string<uint8_t> data_buf((cnt_bits + 7) / 8, 0);
        td::BitPtr data_bit_buf(data_buf.data(), 0);

        unsigned bits_left = cnt_bits;
        while (bits_left > 0) {
            uint8_t cur_bits = std::min(bits_left, 8u);
            uint8_t byte = cell_slice.fetch_ulong(cur_bits);
            bits_left -= cur_bits;
            if (is_special && data_bit_buf.offs == 0) {
                CHECK(cur_bits >= 8);
                CHECK(byte != 0);
                --byte;
                CHECK(byte < 4);
                data_bit_buf.store_uint(byte, 2);
                data_bit_buf.offs += 2;
                cnt_bits -= 6;
            } else {
                data_bit_buf.store_uint(byte, cur_bits);
                data_bit_buf.offs += cur_bits;
            }
        }
        data = std::basic_string<uint8_t>(data_bit_buf.ptr, (cnt_bits + 7) / 8);

        const unsigned cnt_refs = cell_loaded.data_cell->get_refs_cnt();
        CHECK(cnt_refs <= 4);
        for (unsigned i = 0; i < cnt_refs; i++) {
            td::Ref<vm::Cell> ref = cell_loaded.data_cell->get_ref(i);
            refs_as_cells[i] = ref;
        }

        if (cell_type == CellType::MerkleUpdate || cell_type == CellType::MerkleProof) {
            data.resize(1);
            cnt_bits = 2;
        }
    }

    uint8_t d1() const {
        return cnt_refs + 8 * is_special;
    }

    uint8_t d2() const {
        unsigned desc = cnt_bits / 8;
        desc *= 2;
        if (cnt_bits % 8 != 0) {
            desc ^= 1u;
        }
        return desc;
    }

    bool operator==(const MyCell& rhs) const {
        if (cnt_bits != rhs.cnt_bits)
            return false;
        if (data != rhs.data)
            return false;
        if (is_special != rhs.is_special)
            return false;
        if (cnt_refs != rhs.cnt_refs)
            return false;
        if (refs != rhs.refs)
            return false;
        return true;
    }

    // void print() const {
    //     std::cout << "MyCell|cnt_bits=" << cnt_bits;
    //     std::cout << ", is_special=" << is_special << ", cnt_refs=" << cnt_refs;
    //     std::cout << ", data=";
    //     for (const auto &c: data) {
    //         std::cout << std::bitset<8>(c) << " ";
    //     }
    //     std::cout << std::dec;
    //     std::cout << ", refs=";
    //     for (const auto &r: refs) {
    //         std::cout << r << " ";
    //     }
    //     std::cout << "|" << std::endl;
    // }
};

std::vector<Ptr<MyCell> > convert_to_my_cells(td::Ref<vm::Cell> root) {
    std::vector<Ptr<MyCell> > my_cells;

    std::unordered_map<const vm::Cell *, unsigned> pos;
    auto dfs = [&](auto&& self, td::Ref<vm::Cell> cell) -> void {
        if (pos.count(cell.get())) {
            return;
        }
        pos[cell.get()] = my_cells.size();
        auto my_cell = std::make_shared<MyCell>(cell);
        my_cells.push_back(my_cell);
        CHECK(my_cell->cnt_refs <= 4);
        for (unsigned i = 0; i < my_cell->cnt_refs; ++i) {
            self(self, my_cell->refs_as_cells[i]);
        }
    };

    dfs(dfs, root);

    for (const auto& cell: my_cells) {
        for (unsigned i = 0; i < cell->cnt_refs; ++i) {
            td::Ref<vm::Cell> ref = cell->refs_as_cells[i];
            cell->refs[i] = pos.at(ref.get());
        }
    }

    return my_cells;
}

std::basic_string<uint8_t> serialize(td::Ref<vm::Cell> root) {
    auto my_cells = convert_to_my_cells(root);
    const unsigned n = my_cells.size();

    // reorder cells
    {
        std::vector<uint8_t> out_degree(n);
        std::vector<std::vector<unsigned> > who_refers(n);
        for (unsigned i = 0; i < n; i++) {
            out_degree[i] = my_cells[i]->cnt_refs;
            for (unsigned j = 0; j < my_cells[i]->cnt_refs; j++) {
                who_refers[my_cells[i]->refs[j]].push_back(i);
            }
        }

        auto cmp = [&](unsigned i, unsigned j) {
            if (out_degree[i] != out_degree[j]) {
                return out_degree[i] < out_degree[j];
            }
            if (who_refers[i].size() != who_refers[j].size()) {
                return who_refers[i].size() > who_refers[j].size();
            }
            return i < j;
        };

        std::set<unsigned, decltype(cmp)> cells(cmp);
        for (unsigned i = 0; i < n; i++) {
            cells.insert(i);
        }

        std::vector<unsigned> order;
        order.reserve(n);
        while (!cells.empty()) {
            auto it = cells.begin();
            unsigned i = *it;
            cells.erase(it);

            CHECK(out_degree[i] == 0);

            order.push_back(i);

            for (unsigned from: who_refers[i]) {
                cells.erase(from);
                CHECK(out_degree[from] > 0);
                --out_degree[from];
                cells.insert(from);
            }
        }

        // SHUFFLE
        // std::random_device rd;
        // std::mt19937 g(rd());
        // std::shuffle(order.begin(), order.end(), g);
        // std::sort(order.begin(), order.end(), [&](unsigned i, unsigned j) {
        //     return my_cells[i]->cnt_bits < my_cells[j]->cnt_bits;
        // });

        // update ref indexes
        std::vector<unsigned> pos(n);
        for (unsigned i = 0; i < n; i++) {
            pos[order[i]] = i;
        }

        for (auto& my_cell: my_cells) {
            for (unsigned i = 0; i < my_cell->cnt_refs; i++) {
                my_cell->refs[i] = pos[my_cell->refs[i]];
            }
        }

        // physically reorder cells
        {
            std::vector<Ptr<MyCell> > new_my_cells(n);
            for (unsigned i = 0; i < n; i++) {
                new_my_cells[pos[i]] = my_cells[i];
            }
            my_cells.swap(new_my_cells);
        }
    }

    std::basic_string<uint8_t> bytes(50 + n + n * 4 * 3 + n * 256, 0);
    td::BitPtr bits_ptr(bytes.data(), 0);
    huffman::HuffmanEncoderDecoder d1_encoder(huffman::d1_freq);
    huffman::HuffmanEncoderDecoder data_size_encoder(huffman::data_size_freq);
    std::basic_string<unsigned> cell_id;

    // store n
    bits_ptr.store_uint(n, 20);
    bits_ptr.offs += 20;

    // compress refs
    const bool compress_refs = false;
    bits_ptr.store_uint(compress_refs, 1);
    bits_ptr.offs += 1;

    // d1
    for (unsigned i = 0; i < n; ++i) {
        const auto d1 = my_cells[i]->d1();
        auto spent_bits = d1_encoder.encode_symbol(bits_ptr, d1);
        cell_id += std::basic_string<unsigned>(spent_bits, i);
    }

    // data sizes
    for (unsigned i = 0; i < n; ++i) {
        const auto data_size = my_cells[i]->cnt_bits;
        auto spent_bits = data_size_encoder.encode_symbol(bits_ptr, data_size);
        cell_id += std::basic_string<unsigned>(spent_bits, i);
    }

    // refs
    for (unsigned i = 0; i < n; ++i) {
        const auto bit_len = compress_refs ? len_in_bits(i) : len_in_bits(n - 1);
        for (unsigned j = 0; j < my_cells[i]->cnt_refs; ++j) {
            const unsigned ref = my_cells[i]->refs[j];
            // CHECK(ref < i);

            const auto ref_store = (n + i - ref) % n; //rd();
            bits_ptr.store_uint(ref_store, bit_len);
            bits_ptr.offs += bit_len;

            cell_id += std::basic_string<unsigned>(bit_len, i);
        }
    }

    const unsigned data_start = bits_ptr.offs;
    // data
    for (unsigned i = 0; i < n; ++i) {
        const auto my_cell = my_cells[i];
        td::ConstBitPtr data_ptr(my_cell->data.data(), 0);
        for (unsigned j = 0; j < my_cell->cnt_bits; ++j) {
            bool bit = data_ptr.get_uint(1);
            data_ptr.offs += 1;
            bits_ptr.store_uint(bit, 1);
            bits_ptr.offs += 1;
        }
        cell_id += std::basic_string<unsigned>(my_cell->cnt_bits, i);
    }
    const unsigned data_end = bits_ptr.offs;

    const auto output = std::basic_string<uint8_t>(bits_ptr.ptr, (bits_ptr.offs + 7) / 8);

    // check lz references
    if (false) {
        const auto is_son = [&](auto&& self, const unsigned par, const unsigned son) -> bool {
            for (unsigned i = 0; i < my_cells[par]->cnt_refs; ++i) {
                const auto ref = my_cells[par]->refs[i];
                if (ref == son || self(self, ref, son)) {
                    return true;
                }
            }
            return false;
        };

        const auto bits = bytes_str_to_bit_str(output);
        const unsigned n = bits.size();
        std::vector<unsigned> best_match_suff(n, -1);
        std::vector<unsigned> best_match_len(n, 0);
        // suff aut
        {
            SuffAut<2> suff_aut_bits(bits);
            suff_aut_bits.build_matches(best_match_suff, best_match_len, lz_compressor_bits::MIN_MATCH_LENGTH);
        }

        std::map<unsigned, unsigned> cnt_with_matches_x_cells;
        std::map<unsigned, unsigned> sum_len_matches;
        std::set<unsigned> ids_involved;
        std::set<unsigned> ids_referenced;
        std::set<std::basic_string<uint8_t> > match_prefs;
        const unsigned pref_th = 64;
        unsigned cnt_matches = 0;
        std::map<unsigned, unsigned> cell_offsets;
        unsigned cnt_interleaved_matches = 0;
        unsigned sum_matches_len = 0;
        unsigned cnt_matches_with_sons = 0;
        DSU dsu(my_cells.size());
        unsigned cnt_wallet_cells_who_reference = 0;
        unsigned cnt_wallet_cells_who_reference_in_data = 0;
        unsigned cnt_wallet_cells_who_reference_other_wallet_cells = 0;
        for (unsigned i = 0; i < n;) {
            const auto match_len = best_match_len[i];
            if (match_len >= lz_compressor_bits::MIN_MATCH_LENGTH) {
                const auto start = best_match_suff[i];
                if (start + match_len > i) {
                    ++cnt_interleaved_matches;
                }
                if (my_cells[cell_id[i]]->cnt_bits == 282) {
                    ++cnt_wallet_cells_who_reference;
                    if (data_start <= i && i < data_end) {
                        ++cnt_wallet_cells_who_reference_in_data;
                    }
                }
                std::set<unsigned> diff_ids;
                bool match_son = false;
                for (unsigned j = 0; j < match_len; ++j) {
                    const auto id_ref = cell_id[start + j];
                    if (is_son(is_son, cell_id[i], id_ref)) {
                        match_son = true;
                    }
                    dsu.unite(cell_id[i], id_ref);
                    diff_ids.insert(id_ref);
                    CHECK(bits[start + j] == bits[i + j]);
                }
                if (my_cells[cell_id[i]]->cnt_bits == 282) {
                    bool ref_other_wallet = false;
                    for (auto id: diff_ids) {
                        if (my_cells[id]->cnt_bits == 282) {
                            ref_other_wallet = true;
                        }
                    }
                    cnt_wallet_cells_who_reference_other_wallet_cells += ref_other_wallet;
                }
                cnt_matches_with_sons += match_son;
                unsigned cell_offset = 0;
                while (start >= cell_offset && cell_id[start] == cell_id[start - cell_offset]) {
                    ++cell_offset;
                }
                cell_offsets[cell_offset] += 1;
                cnt_with_matches_x_cells[diff_ids.size()] += 1;
                sum_len_matches[diff_ids.size()] += match_len;

                const unsigned pref_len = std::min(pref_th, match_len);
                match_prefs.insert(bits.substr(start, pref_len));

                ids_involved.insert(cell_id[i]);
                for (auto id: diff_ids) {
                    ids_involved.insert(id);
                    ids_referenced.insert(id);
                }
                cnt_matches += 1;

                sum_matches_len += match_len;

                i += match_len;
            } else {
                ++i;
            }
        }
        unsigned sum_len_referenced_cells = 0;
        for (const auto id: ids_referenced) {
            if (id != -1u) {
                sum_len_referenced_cells += my_cells[id]->cnt_bits;
            }
        }
        std::cout << "stats for MATCHING other cells: " << std::endl;
        for (const auto& it: cnt_with_matches_x_cells) {
            std::cout << it.second << " matches referenced " << it.first << " different other cells, "
                    << "avg len is " << sum_len_matches[it.first] * 1.0 / it.second << std::endl;
        }
        std::cout << ids_involved.size() << " cells involved in matches out of "
                << my_cells.size() << " cells" << std::endl;
        std::cout << "there are " << match_prefs.size() << " different match prefixes of length "
                << pref_th << " out of " << cnt_matches << " matches" << std::endl;
        std::cout << cell_offsets.size() << " different cell offsets (up to ~1023), out of "
                << cnt_matches << " matches" << std::endl;
        std::cout << "cnt_interleaved_matches = " << cnt_interleaved_matches << " out of "
                << cnt_matches << " matches" << std::endl;
        std::cout << "sum_matches_len = " << sum_matches_len << ", this is "
                << (sum_matches_len * 100.0 / n) << "% out of all bits" << std::endl;
        std::cout << "sum_len_referenced_cells = " << sum_len_referenced_cells << std::endl;
        std::cout << "cnt referenced cells = " << ids_referenced.size() << std::endl;
        std::cout << "cnt_matches_with_sons = " << cnt_matches_with_sons << std::endl;
        std::cout << "cnt connected comps of cells by refs = " << dsu.cnt_comps() << std::endl;
        std::cout << "comp sizes: " << std::endl;
        std::map<unsigned, unsigned> cnt_with_sz;
        for (const auto sz: dsu.comp_sizes()) {
            cnt_with_sz[sz] += 1;
        }
        for (const auto& it: cnt_with_sz) {
            std::cout << it.second << " comps with sz=" << it.first << std::endl;
        }
        std::cout << "serialized len = " << n << std::endl;

        // for (auto it : cell_offsets) {
        //     std::cout << "cell_offset=" << it.first << ", its cnt=" << it.second << std::endl;
        // }
        std::cout << "cnt_wallet_cells_who_reference = " << cnt_wallet_cells_who_reference << std::endl;
        std::cout << "cnt_wallet_cells_who_reference_in_data = " << cnt_wallet_cells_who_reference_in_data << std::endl;
        std::cout << "cnt_wallet_cells_who_reference_other_wallet_cells = "
                << cnt_wallet_cells_who_reference_other_wallet_cells << std::endl;

        std::map<unsigned, unsigned> cnt_with_data_sz;
        for (const auto& cell: my_cells) {
            cnt_with_data_sz[cell->cnt_bits] += 1;
            // if (cell->cnt_bits == 282) {
            //     td::BitPtr ptr(cell->data.data(), 0);
            //     std::cout << std::hex << "0x" << ptr.get_uint(4 * 8) << std::dec << std::endl;
            // }
        }
        // std::cout << "static const std::vector<unsigned> data_size_freq = {";
        // for (const auto &it: cnt_with_data_sz) {
        //     std::cout << it << ",";
        // }
        // std::cout << "};" << std::endl;
        // exit(0);
        // for (const auto& it: cnt_with_data_sz) {
        //     std::cout << it.second << " cells with data sz=" << it.first << std::endl;
        // }
        // exit(0);
    }

    return output;
}

td::BufferSlice serialize_slice(td::Ref<vm::Cell> root) {
    auto bytes = serialize(root);
    return td::BufferSlice(reinterpret_cast<const char *>(bytes.data()), bytes.size());
}

td::Ref<vm::Cell> deserialise(std::basic_string<uint8_t> buffer) {
    td::BitPtr buffer_bit(buffer.data(), 0);
    huffman::HuffmanEncoderDecoder d1_decoder(huffman::d1_freq);
    huffman::HuffmanEncoderDecoder data_size_decoder(huffman::data_size_freq);

    const unsigned n = buffer_bit.get_uint(20);
    buffer_bit.offs += 20;

    const bool compress_refs = buffer_bit.get_uint(1);
    buffer_bit.offs += 1;

    std::vector<Ptr<MyCell> > my_cells;
    my_cells.reserve(n);

    // build my_cells
    for (unsigned i = 0; i < n; ++i) {
        Ptr<MyCell> my_cell = std::make_shared<MyCell>();

        const auto d1 = d1_decoder.decode_symbol(buffer_bit);
        my_cell->cnt_refs = d1 & 7;
        CHECK(my_cell->cnt_refs <= 4);
        my_cell->is_special = d1 & 8;

        my_cells.push_back(my_cell);
    }

    // data sizes
    for (auto& my_cell: my_cells) {
        const auto data_size = data_size_decoder.decode_symbol(buffer_bit);
        my_cell->cnt_bits = data_size;
    }

    // read refs
    for (unsigned i = 0; i < n; ++i) {
        auto& my_cell = my_cells[i];

        const auto bit_len = compress_refs ? len_in_bits(i) : len_in_bits(n - 1);
        for (unsigned j = 0; j < my_cell->cnt_refs; j++) {
            unsigned ref_id = (n + i - buffer_bit.get_uint(bit_len)) % n;
            buffer_bit.offs += bit_len;
            my_cell->refs[j] = ref_id;
            if (compress_refs) {
                CHECK(ref_id < i);
            }
        }
    }

    for (unsigned i = 0; i < n; ++i) {
        auto& my_cell = my_cells[i];
        my_cell->data.resize((my_cell->cnt_bits + 7) / 8, 0);
        td::BitPtr my_cell_bit_ptr(my_cell->data.data(), 0);
        for (unsigned j = 0; j < my_cell->cnt_bits; ++j) {
            my_cell_bit_ptr.store_uint(buffer_bit.get_uint(1), 1);
            buffer_bit.offs += 1;
            my_cell_bit_ptr.offs += 1;
        }
    }

    // top sort, just in case
    std::vector<unsigned> order;
    {
        std::vector<char> was(n, 0);
        const auto dfs = [&](auto&& self, unsigned v) -> void {
            CHECK(v < n);
            CHECK(!was[v]);
            was[v] = true;
            for (unsigned i = 0; i < my_cells[v]->cnt_refs; ++i) {
                const auto to = my_cells[v]->refs[i];
                if (!was[to]) {
                    self(self, to);
                }
            }
            order.push_back(v);
        };
        for (unsigned v = 0; v < n; ++v) {
            if (!was[v]) {
                dfs(dfs, v);
            }
        }
    }
    CHECK(order.size() == n);

    // build DAG of td::Ref<vm::Cell>
    std::vector<td::Ref<vm::Cell> > cells(n);
    std::vector<char> ready(n, 0);
    for (const auto i : order) {
        const auto& my_cell = my_cells.at(i);

        vm::CellBuilder cell_builder;

        // add refs
        std::vector<td::Ref<vm::Cell> > refs;
        for (unsigned j = 0; j < my_cell->cnt_refs; ++j) {
            unsigned ref_id = my_cell->refs[j];
            // CHECK(len_in_bits(ref_id) <= len_in_bits(i));
            CHECK(cells[ref_id].get());
            CHECK(ready[ref_id]);
            cell_builder.store_ref(cells[ref_id]);
            refs.push_back(cells[ref_id]);
        }

        const auto cell_type = my_cell->get_type();
        std::basic_string<uint8_t> data = my_cell->data;;
        unsigned cnt_bits = my_cell->cnt_bits;
        if (my_cell->is_special) {
            const uint8_t cell_type_int = (data[0] >> 6) + 1;
            cnt_bits += 6;

            std::basic_string<uint8_t> new_data((cnt_bits + 7) / 8, 0);
            td::BitPtr new_data_ptr(new_data.data(), 0);
            new_data_ptr.store_uint(cell_type_int, 8);
            new_data_ptr.offs += 8;

            td::BitPtr data_ptr(data.data(), 2);
            unsigned new_data_bits = new_data_ptr.offs;
            push_bit_range(new_data, new_data_bits, data_ptr, 2, cnt_bits - 8);

            data = new_data;
            CHECK(new_data_bits == cnt_bits);
            cnt_bits = new_data_bits;
        }
        if (cell_type == CellType::MerkleUpdate || cell_type == CellType::MerkleProof) {
            // hash0 (256), hash1, depth0 (8 * bytes_for_index), depth1
            CHECK(refs.size() <= 2);
            CHECK(data.size() == 1);
            CHECK(cnt_bits == 8);
            const uint8_t bytes_for_index_max = len_in_bytes(n);

            data.resize(1 + 256 / 8 * 2 + 8 * bytes_for_index_max, 0);
            td::BitPtr data_ptr(data.data(), 8);

            for (unsigned j = 0; j < refs.size(); ++j) {
                const auto hash = refs[j]->get_hash(0);
                const auto slice = hash.as_slice();
                for (unsigned k = 0; k < 256 / 8; ++k) {
                    data_ptr.store_uint(slice.data()[k], 8);
                    data_ptr.offs += 8;
                }
            }

            for (unsigned j = 0; j < refs.size(); ++j) {
                unsigned depth = refs[j]->get_depth(0);
                data_ptr.store_uint(depth, bytes_for_index_max * 8);
                data_ptr.offs += bytes_for_index_max * 8;
            }
            cnt_bits = data_ptr.offs;
        }

        // add data
        cell_builder.store_bits(
            data.data(),
            cnt_bits
        );

        td::Ref<vm::DataCell> cell{};
        auto finalised = cell_builder.finalize_novm_nothrow(my_cell->is_special);
        cell = finalised.move_as_ok();
        CHECK(cell->is_special() == my_cell->is_special);
        CHECK(cell->get_bits() == cnt_bits);

        cells[i] = cell;
        ready[i] = true;
    }

    return cells[order.back()];
}

td::Ref<vm::Cell> deserialise_slice(td::Slice buffer_slice) {
    std::basic_string<uint8_t> buffer(buffer_slice.data(), buffer_slice.data() + buffer_slice.size());
    return deserialise(buffer);
}

std::string compress(
    const std::string& base64_data,
    bool return_before_huffman = false,
    bool return_after_serialize = false
) {
    CHECK(!base64_data.empty());
    td::BufferSlice data(td::base64_decode(base64_data).move_as_ok());
    td::Ref<vm::Cell> root = vm::std_boc_deserialize(data).move_as_ok();

    auto S = serialize(root);
    if (return_after_serialize) {
        return to_string(S);
    }
    // CHECK(!uncompressed_dict.empty());
    S = lz_compressor_bits::compress(S, uncompressed_dict);

    if (return_before_huffman) {
        return to_string(S);
    }
    S = huffman::encode_8(S);
    auto base64 = td::base64_encode(td::Slice(S.data(), S.size()));
    return base64;
}

std::string decompress(const std::string& base64_data) {
    CHECK(!base64_data.empty());
    std::string data = td::base64_decode(base64_data).move_as_ok();
    std::basic_string<uint8_t> S(data.begin(), data.end());
    S = huffman::decode_8(S);

    S = lz_compressor_bits::decompress(S, uncompressed_dict);

    td::Ref<vm::Cell> root = deserialise(S);
    td::BufferSlice serialised_properly = vm::std_boc_serialize(root, 31).move_as_ok();
    return base64_encode(serialised_properly);
}
};
