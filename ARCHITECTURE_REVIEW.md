# LProject 아키텍처 자가 진단 (Self-Review)

> **작성일:** 2026-06-20
> **방법:** `Source/` 전체(약 6,350줄 / 80개 파일)를 7개 차원으로 나눠 **읽기 전용** 멀티에이전트 리뷰 + 독립 적대적 종합. 코드는 수정하지 않았음.
> **목적:** "로스트아크 보스전을 완벽하게 구현한다"는 목표 대비, 지금까지 만든 구조의 실제 수준을 가감 없이 진단.

---

## 한 줄 결론

**시스템 골격으로는 B-, "완벽한 로아 보스전"이라는 기준으로 보면 C/D.**
넓이(breadth)는 다 깔았는데 깊이(depth)가 없다. 그리고 그걸 "완성"이라고 부른 것이 문제였다.

| 차원 | 등급 |
|---|---|
| 전체 아키텍처 / 모듈 구성 | **B** |
| GAS 구현 정확성 / 관용성 | **B** |
| 보스 + 인카운터 시스템 | **C** |
| C++ UI (UMG, no WBP) | **C** |
| 게임 플로우 / 상태머신 | **C** |
| 캐릭터 / 입력 배선 | **C** |
| "진짜 로아 보스전인가" 갭 분석 | **C** (목표 기준 D) |
| **종합** | **C** |

---

## 1. 진짜 잘 된 부분 (검증됨, 허세 아님)

- **GAS 데미지 심이 실제로 하나로 통일됨.** 평타, QWER 스킬 4개, 카운터, 출혈 DoT, 보스 패턴이 *전부* 단일 `ULProjectExecCalc_Damage`로 흐른다. HP를 깎는 우회 경로가 없음. 제일 틀리기 쉬운 걸 맞게 했다.
- **메타 어트리뷰트가 교과서적으로 정확.** `Damage` meta-attribute는 `HideFromModifiers` + 복제 제외(`AttributeSet.cpp:30`), `PostGameplayEffectExecute`에서 소비/0화/클램프. 클램프는 `PreAttributeChange`라는 올바른 자리에.
- **쿨다운이 관용적.** `ULProjectGA_Skill`이 `GetCooldownTags()`/`ApplyCooldown()`을 오버라이드, 스킬별 태그를 `DynamicGrantedTags`에 주입하고 단일 공용 `GE_Cooldown`을 SetByCaller 듀레이션으로 구동(`GA_Skill.cpp:35-52`). Lyra식.
- **소유권/수명 판단이 건전.** 플로우는 월드 리셋을 견디는 `GameInstanceSubsystem`, 디렉터는 월드 단위 `WorldSubsystem`. 텔레그래프/스트라이크 분리(경고 액터는 순수 비주얼, 권위 오버랩은 러너가).
- **그로기 루프가 끝까지 배선됨.** 스태거 0 → 러너 일시정지 + Groggy 태그 → ExecCalc 2배 → 타이머 리필. 알아볼 수 있는 로아 서브 기믹이 실제로 구현됨.

여기까진 그레이박스 평균 이상. **문제는 이 위.**

---

## 2. 치명적 갭 3개 — "퀄리티 낮다"의 정체

전부 `grep` 0건. 즉 **스텁조차 없음.**

### 2-1. 애니메이션 기반 전투가 아예 없음 `critical`
- `Montage` / `RootMotion` / `HitReact` 검색 0건.
- 모든 공격이 활성화 즉시 `OverlapMultiByObjectType` 한 번 치는 즉발 트레이스 (`GA_BasicAttack.cpp:43`). 헤더에도 *"instant trace on activation, no montage"* 라고 적혀 있음.
- 캐릭터 메시는 속도값으로 idle/walk/run만 단일 노드 루프. 공격 애니메이션 없음, 히트 윈도우를 여는 anim notify 없음.
- **결과:** 보스 몸에서 읽을 윈드업이 없다. 땅에 깔리는 디버그 도형이 유일한 예고. 로아 전투의 본질이 통째로 빠짐.

### 2-2. 타격 피드백 레이어 전무 `critical`
- `CameraShake` / `HitStop` / `TimeDilation` / `Niagara` / `SpawnEmitter` / `PlaySound` / `DamageNumber` 검색 0건.
- 데미지는 `PostGameplayEffectExecute`에서 숫자만 조용히 깎임. HP 바 외엔 피격 확인 수단 없음.
- **`Tex_DamageText_Impact` 텍스처는 만들어 놓고 코드에서 한 번도 안 씀.** 에셋만 있고 시스템이 없다.

### 2-3. 보스 기믹 어휘가 1개뿐 `critical`
- 전체 보스 무브셋 = 텔레그래프 후 오버랩 AoE, 3가지 도형(Circle/Box/Cone) × 3가지 기준(Player/Self/Forward).
- 넉백·풀·그랩·스택·스프레드·세이프존·쫄(adds)·색깔/시계방향 기믹·와이프 기믹 전부 없음.
- **보스는 이동 코드가 0.** `Tick`에서 플레이어 향해 yaw 회전만(`BossCharacter.cpp:254-276`). 임포트한 Walk/Run 애니는 사실상 절대 안 나옴.
- **결과:** 최적 플레이가 "최대 근접 거리에서 원만 밟지 말기"로 수렴. 제자리 포탑.

---

## 3. 간판 시스템들이 속 빈 깡통

| 시스템 | 문서상 | 실제 |
|---|---|---|
| **페이즈** | 100/66/33% HP 게이트 | 기본 패턴에 `RequiredPhaseTags` 미설정 → **1/2/3 페이즈가 완전히 동일하게 플레이**됨. 전환 = HUD 글자만 바뀜 |
| **인레이지** | "DPS 체크" | 그냥 300초 스톱워치. 보스 HP를 안 봄. `TAG_State_Boss_Enraged`는 **선언만 하고 적용 0건** |
| **파트브레이크** | 부위별 약점 | 부위별 `ApplyPartDamage` **호출자 0**. 그냥 배열 순서대로 닳는 2차 HP 게이트 |

근거: `EncounterDirector.cpp:196-209`, `BossPatternRunnerComponent.cpp:340-387`, `PartBreakComponent.cpp:37-56`.

---

## 4. 진짜 버그 / 구조 문제

- **상태머신이 가드 없는 단순 대입.** `RequestState`가 `CurrentState = NewState;`(`GameFlowSubsystem.cpp:111`)뿐. 합법 전환 테이블 없음.
  - **R키를 타이틀 화면에서 눌러도 바로 전투 진입**(`PlayerController.cpp:24-38`이 무조건 `Flow->Retry()`).
  - `bBoundEnded` 플래그가 한 번 set 후 **리셋이 안 됨** → **PIE 재시작하면 새 디렉터의 `OnEncounterEnded`가 다시 안 바인딩되어 결과화면이 영영 안 뜸**. `Initialize/Deinitialize` 자체가 없음.
- **"데이터 드리븐"인데 실행 경로에 소비자가 없음.** PawnData 에셋이 없어서 매 실행마다 `EnsureDefaultPawnData()`가 InputAction 8개 + IMC + InputConfig + PawnData를 코드로 재구성하고, 능력 리스트는 `GrantAbilities()`에 한 번 더 하드코딩. **같은 킷을 두 군데에 손으로 동기화**해야 함. AbilitySet의 어트리뷰트 부여 경로는 플레이어도 보스도 안 씀.
- **죽은 코드.** `ILProjectCombatant` 인터페이스 — 디커플링 심이라 문서화돼 있지만 `Cast<ILProjectCombatant>`/`TScriptInterface` 사용처 0건. `ALProjectDebugHUD` — RaidHUD와 같은 readout을 두 번 구현했는데 `SetHUD`/`HUDClass`로 참조하는 곳이 없음.
- **Counter가 거꾸로 의존.** 저수준 전투 능력이 `EncounterDirector`/`BossCharacter` 콘크리트 타입을 직접 잡음(`GA_Counter.cpp:9-63`). 레이어가 뒤집혔고, "디렉터의 그 보스" 외엔 카운터 불가. 프로젝트 최대 결합 냄새.
- **i-frame / 자원이 손수 구현.** Identity를 GE가 아니라 `SetNumericAttributeBase` 직접 호출로 변경(예측·큐·MP 동기화 없음). Cost GE 없어서 `CanActivateAbility`가 Identity 게이트를 반영 못함. i-frame은 비복제 loose 태그 + 맨 타이머. 그런데 모든 능력은 `LocalPredicted` 선언 → "예측된다"는 주석이 거짓.
- **CLAUDE.md가 양방향으로 틀림.** "WASD"라는데 실제 RMB 클릭 무브, "Q 카운터"라는데 실제 F(HUD 라벨은 F로 맞음), "Content 비었음"인데 114개 파일, "DebugHUD를 HUD로 설정"인데 설정 안 함, "DPS 체크"인데 스톱워치. 스킬/Identity/UMG HUD/스크린 플로우 레이어는 통째로 누락.

---

## 5. 근본 원인

> **로아 레이드가 "가졌다고 묘사되는" 모든 시스템의 가로 골격은 다 깔았지만, 각 시스템이 "기본값 1개로 도는 데이터 경로"가 되는 지점에서 멈췄고, 보스전을 *보스전처럼 느끼게* 만드는 세로 깊이(애니메이션·피드백·실제 기믹·실 콘텐츠)는 안 만든 채 '완성'이라 불렀다.**

그래서 코드는 "전부 다 있음"으로 읽히는데 플레이는 "기믹 1개, 손맛 0"로 느껴진다. 위화감의 정체가 이 충돌이다.

---

## 6. 권장 다음 단계

**해법은 시스템을 더 늘리는 게 아니다.** 핵심 루프 1개를 골라 **수직으로 "진짜"처럼** 만든다:

> **텔레그래프 → 회피 → 카운터 → 그로기 → 버스트**

이 한 루프에:
1. **몽타주 기반 공격** (윈드업/커밋/회복, anim notify로 히트 윈도우)
2. **히트스톱 + 카메라 셰이크** (피격 임팩트)
3. **플로팅 데미지 숫자** (이미 있는 `Tex_DamageText_Impact` 사용)
4. **움직이는 보스** (이동/갭클로저/리포지션)
5. **피격 VFX/SFX**

그게 되면 나머지 갭(페이즈 기믹, 기믹 어휘, 인레이지 DPS 체크)이 뭘 채워야 하는지 분명해진다.

**병행 권장(저비용·고효과 정리):** 상태머신 전환 가드 + `Deinitialize`로 `bBoundEnded`/위젯 정리, 입력 데이터 경로 단일화, 죽은 코드(`ILProjectCombatant`·`DebugHUD`) 처리, CLAUDE.md 현행화.

---

# v2 — 프로덕션 패스 후 재평가 (2026-06-20)

> 위 진단(Phase A~D 패스)을 실제로 구현한 뒤, **동일한 멀티에이전트 읽기 전용 리뷰**를 상용 기준으로 다시 돌린 결과. 모든 "수정됨" 항목은 grep/코드로 재검증함.

## 종합: **C → B** (코드 검증된 실질 상승)

| 차원 | v1 | v2 |
|---|---|---|
| 플로우/라이프사이클/구조·죽은코드 | C | **B** |
| GAS 관용성/정확성 | B | **B** |
| 보스 + 인카운터 | C | **B** |
| 타격감(juice) | **F** | **B** |
| UI (C++ UMG) | C | **B** |
| 경험 갭 (로아 보스전 거리) | C (목표 D) | **B** |

한 줄: *"이전엔 속 빈 시스템이었던 게 전부 load-bearing이 됐고 엔지니어링은 상용 그레이박스 기준을 통과한다. 다만 보스 1마리 + 패턴 ~7개 + 100% 절차적 애니/피드백이라, 강한 수직 슬라이스 골격이지 아직 Behemoth급 인카운터는 아니다."*

## 검증된 핵심 개선
- **타격감 F→B (최대 점프):** grep 0건이던 레이어가 단일 데미지 심에서 카메라 셰이크·히트스톱(실시간 복원)·빌보드 데미지 숫자·메시 스쿼시 플린치로 구현됨. 우회 경로 없음.
- **속 빈 보스 시스템이 전부 실체화:** 페이즈가 태그-게이트 무브셋 전환 + 무적 포효, 인레이지가 실제 AttackUp GE + 케이던스 + 태그, 파트브레이크가 부위별 + 실제 호출, 보스가 이동.
- **Identity가 관용적 GAS:** `GE_IdentityGain` 인스턴트 GE + PostExec 재클램프, Identity에 닿는 `SetNumericAttributeBase` 0건(검증). 트루 데미지·쿨다운 태그·i-frame 일관성 전부 적용.
- **플로우 라이프사이클 상용급:** 합법 전환 테이블 + 가드, `Deinitialize`, per-world 디렉터 재바인딩(PIE 소프트락 해결).
- **죽은 코드/역의존 정리:** `DebugHUD` 완전 삭제, `ILProjectCombatant` 폴리모픽 사용, `GA_Counter` 콘크리트 타입 의존 제거.
- **v2 라운드에서 추가 수정:** 카운터 프롬프트 키 `[Q]→[F]`(출하 차단급), 보스가 공격 중 facing 고정(후방 타격 보상 루프 작동), 갭클로저 차지 패턴(아레나 공간 이동), 잔존 죽은 코드/주석 정리.

## 남은 거리 (정직하게)
**엔지니어링 부족 (내가 더 할 수 있음):**
- 기믹 어휘가 여전히 얇음 — 쫄 소환/그랩/스택·스프레드/테더/와이프 등 멀티액터·스크립트 set-piece 없음. (데이터 모델에 합성 패턴 표현 불가) — *raid다움의 핵심, 콘텐츠/엔지니어링 작업*
- GAS 폴리시(Cost GE 없음, loose-tag i-frame, LocalPredicted 라벨 부정확) — SP에선 무해하나 A를 막는 폴리시

**저자 에셋 필요 (헤드리스 불가, 훅은 준비됨):**
- 애니메이션 기둥이 100% 절차적 — 실제 몽타주/루트모션/anim-notify 히트윈도우 없음 (공격은 여전히 즉발 단일틱 오버랩; 타이밍 훅은 몽타주 모양)
- 임팩트 VFX(Niagara) 없음 + 무음(`HitSound` 훅 비어있음)
- 텔레그래프/숫자가 debug-draw/TextRender 프리미티브 — 저작 머티리얼/데칼 필요

→ **결론:** 골격·시스템·심은 상용 기준을 통과(B). 남은 거리는 **대부분 저자 에셋(애니/VFX/SFX/머티리얼)** 과 **더 깊은 기믹 콘텐츠**이며, 재설계 없이 아티스트·콘텐츠 디자이너가 메울 수 있는 형태로 훅이 준비돼 있다.
