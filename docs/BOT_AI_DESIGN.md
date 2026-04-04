# Advanced Bot AI System — Design Document

## Status
- **Default bots restored** — all custom bot code removed, upstream playerbot files restored
- **Files removed**: `bottactics.cpp`, `bottactics.h` (custom tactical AI)
- **Files restored to upstream**: `playerbot.h`, `playerbot.cpp`, `playerbot_master.cpp`, `playerbot_rotation.cpp`, `playerbot_strategy.h`, `playerbot_strategy.cpp`, `g_bot.h`, `g_bot.cpp`
- **Non-bot custom code preserved**: `g_scriptevents`, `curlworker`, `dapserver`, `scriptyaml`, etc.

---

## Current Default Bot Capabilities (upstream)

### What they CAN do
- Pathfind via Recast/Detour navmesh or legacy PathNode A*
- Basic combat: detect enemies by line-of-sight, aim, fire (burst/continuous), melee fallback
- React to sound events (gunfire, explosions, footsteps) → curious/investigate
- Use doors and ladders
- Jump obstacles and gaps
- Visit attractive nodes (map-placed waypoints)
- Auto-join/respawn/team-select

### What they CANNOT do
- **No objective play** — zero awareness of bombs, flags, ropes, or any gamemode objectives
- **No grenade handling** — can't throw or dodge grenades
- **No squad tactics** — each bot is fully independent
- **No cover usage** — never crouch, never use cover nodes
- **No strategic movement** — idle = random wandering
- **No item seeking** — won't intentionally pick up health/ammo
- **No difficulty scaling** — aim spread is random, not skill-driven
- **No vehicle support**

---

## Architecture: New Advanced Bot System

### Layer Architecture

```
┌──────────────────────────────────────────────────────────┐
│  STRATEGIC LAYER (per-team, updates every ~2s)           │
│  • Objective prioritization (which bomb site? which flag)│
│  • Squad assignment (attack squad / defense squad)       │
│  • Resource allocation (who gets what role)              │
│  Custom Utility AI scorer                                │
├──────────────────────────────────────────────────────────┤
│  TACTICAL LAYER (per-bot, updates every ~500ms)          │
│  • Action selection: attack/defend/flank/plant/cover     │
│  • Position evaluation via influence maps on navmesh     │
│  • Threat assessment and response                        │
│  Utility AI scoring + GOAP planner                       │
├──────────────────────────────────────────────────────────┤
│  BEHAVIOR LAYER (per-bot, updates every frame)           │
│  • Execution of chosen action as behavior tree           │
│  • Patrol sequences, engage sequences, plant-bomb, etc.  │
│  BehaviorTree.CPP v4                                     │
├──────────────────────────────────────────────────────────┤
│  NAVIGATION LAYER (existing, enhance)                    │
│  • Recast/Detour v1.6.0 (already integrated)            │
│  • ENABLE: dtCrowd (multi-agent steering/avoidance)     │
│  • ENABLE: dtTileCache (dynamic obstacle support)       │
│  • ENABLE: Virtual query filter (per-bot traversal cost)│
├──────────────────────────────────────────────────────────┤
│  PERCEPTION LAYER (per-bot, updates every ~100ms)        │
│  • Vision: BSP ray traces with FOV + distance            │
│  • Hearing: sound event propagation                      │
│  • Memory: last-known enemy positions, teammate positions│
│  • Spatial queries: KD-tree for fast neighbor lookup     │
└──────────────────────────────────────────────────────────┘
```

---

## External Libraries to Integrate

### Required (new dependencies)

| Library | Version | License | Purpose | Integration |
|---------|---------|---------|---------|-------------|
| **BehaviorTree.CPP** | v4.x | MIT | Behavior tree execution framework | CMake `FetchContent` or submodule in `code/thirdparty/` |

### Already Available (enable/wire up)

| Component | Status | Action Needed |
|-----------|--------|---------------|
| **Recast/Detour v1.6.0** | Active for pathfinding | Already working |
| **dtCrowd** | Compiled but unused | Wire up for multi-bot local avoidance |
| **dtTileCache** | Forward-declared only | Enable for dynamic obstacles (doors, destruction) |
| **Virtual query filter** | Disabled | Enable cmake option `DT_VIRTUAL_QUERYFILTER` |
| **dtPathQueue** | Available | Enable for async/timesliced pathfinding |

### Custom Code (small focused modules)

| Module | Lines (est.) | Purpose |
|--------|-------------|---------|
| Utility AI scorer | ~500 | Response-curve based scoring for action selection |
| GOAP planner | ~800 | Goal-oriented action planning for objective sequencing |
| Influence maps | ~600 | Threat/friendly/objective heatmaps on navmesh polys |
| Squad manager | ~400 | Team coordination, role assignment |
| Objective awareness | ~300 | Detect and interact with gametype entities |

---

## Objective Map Support

### Game Modes to Support

| Mode | Key Entities | Bot Actions Required |
|------|-------------|---------------------|
| **TOW (Tug of War)** | `Tow_Entities` — capture points | Move to point, hold position, defend |
| **Objective (OBJ)** | Bomb targets, breach points | Plant bomb, defend bomb, breach walls |
| **Liberation** | Prisoner rescue zones | Move to zone, escort, defend |
| **Demolition** | Destructible objectives | Attack/defend specific targets |

### Objective Behavior Trees (examples)

**Attack Bomb Site:**
```
Sequence
├── FindNearestObjective (utility score)
├── Fallback
│   ├── Sequence [direct approach]
│   │   ├── IsPathClear?
│   │   └── MoveTo(objectivePos)
│   └── Sequence [tactical approach]
│       ├── FindFlankRoute(objectivePos)
│       ├── RequestSmoke(squadLeader)
│       └── MoveTo(flankPos)
├── Sequence [plant]
│   ├── IsAtObjective?
│   ├── CheckClear(360°)
│   └── UsePlant()
└── Sequence [cover planted bomb]
    ├── FindCoverNear(objectivePos, 300u)
    ├── MoveTo(coverPos)
    └── DefendUntil(bombExplodes)
```

**Defend Bomb Site:**
```
Fallback
├── Sequence [defuse if planted]
│   ├── IsBombPlanted?
│   ├── SuppressEnemies(teammates)
│   └── Defuse()
├── Sequence [hold position]
│   ├── FindDefensePosition(objectivePos)
│   ├── MoveTo(defensePos)
│   └── ReactiveSelector
│       ├── Sequence [engage enemy]
│       │   ├── IsEnemyVisible?
│       │   └── Engage()
│       └── Sequence [watch angles]
│           └── ScanSectors()
```

---

## Influence Map Design

Built directly on navmesh polygons (using `dtPolyRef` as keys):

```
per-poly float arrays:
  threat[]      — enemy presence, decays over time, propagates through adjacency
  friendly[]    — ally presence, same propagation
  objective[]   — distance-weighted priority to current active objective
  danger[]      — recent damage/death locations, grenade landing zones
  visibility[]  — pre-computed exposure score (how many sightlines to poly)
```

**Position evaluation** = weighted sum:
```
score = w_cover * (1 - threat[poly])
      + w_objective * objective[poly]
      + w_safety * (1 - danger[poly])
      + w_team * friendly[poly]
      - w_exposure * visibility[poly]
```

Weights vary by role: flanker maximizes (low threat + high objective), defender maximizes (low exposure + covers objective).

---

## Squad Coordination

```
SquadManager (per team):
  roles: ASSAULT, SUPPORT, FLANKER, SNIPER, DEFENDER
  assignments updated every ~5s based on:
    - objective state (attacking vs defending)
    - bot count (more bots → more specialized roles)
    - bot skill levels
    - current bot health/ammo
  
  Squad comms via shared blackboard:
    - "enemy_spotted" → position + timestamp
    - "need_cover" → requesting bot + position
    - "objective_status" → planted/defused/captured
    - "requesting_smoke" → bot + target area
```

---

## Implementation Phases

### Phase 1: Foundation (BehaviorTree.CPP integration)
- [ ] Add BehaviorTree.CPP v4 as submodule in `code/thirdparty/`
- [ ] Create `BotBrain` class wrapping BT::Tree with blackboard
- [ ] Port existing 5-state system to equivalent behavior tree
- [ ] Verify parity: bots behave exactly like default with new framework
- [ ] Add XML tree definitions for hot-reloading

### Phase 2: Enhanced Navigation
- [ ] Wire up `dtCrowd` for multi-bot local avoidance
- [ ] Enable virtual query filter (per-bot traversal costs)
- [ ] Add basic influence map on navmesh polys (threat + friendly)
- [ ] Implement cover-position finding (trace from navmesh poly to enemy position)

### Phase 3: Combat Improvements
- [ ] Skill-scaled aim (reaction time, accuracy, target leading)
- [ ] Grenade throwing with ballistic calculation
- [ ] Grenade avoidance (detect projectile, flee)
- [ ] Crouching behind cover
- [ ] Weapon selection based on range/situation (Utility AI)
- [ ] Suppressive fire behavior

### Phase 4: Objective Play
- [ ] Objective entity detection (scan for bomb targets, capture points, etc.)
- [ ] Per-gamemode behavior trees (OBJ attack, OBJ defend, TOW, Liberation)
- [ ] Objective-aware strategic layer (which site to attack/defend)
- [ ] GOAP planner for chaining: move → smoke → plant → cover

### Phase 5: Team Coordination
- [ ] Squad manager with role assignment
- [ ] Shared blackboard for team communication
- [ ] Coordinated pushes (wait for teammates before attacking)
- [ ] Callout system (enemy spotted, need backup, grenade out)

### Phase 6: Polish
- [ ] Difficulty profiles (easy/medium/hard/expert) affecting all layers
- [ ] Dynamic difficulty adjustment based on score differential
- [ ] Vehicle awareness
- [ ] Item seeking (health/ammo when low)
- [ ] Personality types (aggressive, cautious, sniper, support)

---

## File Structure (planned)

```
code/fgame/bot/
├── bot_brain.h/.cpp           — BotBrain class, BT tree management
├── bot_perception.h/.cpp      — Vision, hearing, memory
├── bot_influence.h/.cpp       — Influence maps on navmesh
├── bot_squad.h/.cpp           — Squad coordination
├── bot_objective.h/.cpp       — Objective detection & interaction
├── bot_utility.h/.cpp         — Utility AI scorer
├── bot_goap.h/.cpp            — GOAP planner (Phase 4)
├── bot_combat.h/.cpp          — Enhanced combat (aim, grenades, cover)
├── bt_nodes/                  — Custom BT node implementations
│   ├── bt_movement.h/.cpp     — MoveTo, FindCover, Patrol
│   ├── bt_combat.h/.cpp       — Engage, Suppress, ThrowGrenade
│   ├── bt_objective.h/.cpp    — PlantBomb, Defuse, CapturePoint
│   ├── bt_perception.h/.cpp   — CheckEnemy, ScanArea, Investigate
│   └── bt_team.h/.cpp         — RequestSupport, Coordinate
└── trees/                     — XML behavior tree definitions
    ├── basic_combat.xml
    ├── obj_attack.xml
    ├── obj_defend.xml
    ├── tow_capture.xml
    └── idle_patrol.xml
```

---

## Key Design Principles

1. **Layered separation** — strategic decisions don't pollute frame-level execution
2. **Data-driven behavior** — XML trees editable without recompiling
3. **Blackboard communication** — bots share knowledge through typed key-value stores
4. **Navmesh-native tactics** — influence maps built on existing Detour poly topology, no separate grid
5. **Incremental enhancement** — each phase produces working bots with more capability
6. **Existing systems respected** — builds on top of `BotController`/`BotMovement`/`BotRotation`, doesn't replace them
