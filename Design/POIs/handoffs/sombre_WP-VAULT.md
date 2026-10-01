# WP-VAULT — not launched (serial vault, VS-14)

Recorded in VS-07 (2026-10-01), as plan §13.3 requires when the parallel vault package is not used.

- **Decision:** the vault is built **serially by Claude in VS-14**, after the lighthouse (VS-12) and the cable hut (VS-13). There is no `vs/vault` branch and no Grok package.
- **Why:** WP-VAULT runs only if Anthony enables it (plan §13.1), and he has not. The boundary itself was checked against the real scripts in VS-07 and is clean. It is frozen in `Design/POIs/sombre.lighthouse.md`, "Implementation boundary", so enabling the package later costs no redesign.
- **If Anthony enables it** (at Checkpoint B, before VS-14 starts): this file is replaced by a full handoff from `AGENT_HANDOFF_TEMPLATE.md`.
  - **Owns:** the vault row of the boundary table (`vault.py`, `dress_sombre_vault.py`, `Lvl_PointeSombre_Art_Vault`, `Content/World/PointeSombre/Vault/**`, `Tools/Review/Lvl_PointeSombre/vault.json`, `DCSombreVaultMapTest.cpp`).
  - **Forbidden:** `lighthouse.py`, `cable_hut.py`, the core, the ledger, and `DCSombreVaultRoutesMapTest.cpp` (the Integrator's, at the merge).
  - **Stops** if a presence rule would need targets on both sides, or the vault test would need the island.
- **Either way:** the Integrator writes `Map.Sombre.VaultRoutes` once both ends exist, and the vault files are Claude's from VS-15 on.
