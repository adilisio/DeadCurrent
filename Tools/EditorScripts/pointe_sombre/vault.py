"""The vault (inside sombre.lighthouse): the Authority's node under the tower, an abstracted interior cell in the slot
"vault". Spec: Design/POIs/sombre.lighthouse.md (integrated; the vault row of its "Implementation boundary").

PLACEHOLDER (staged by the Integrator, 2026-10-02, VS-10) so the vault package edits only this file. Until it is built,
the greybox's stub vault stands in: its room, its three return portals (Greybox_VaultHatch_In, Greybox_VaultLower_In,
Greybox_VaultConduit_In), and its sombre.vault location volume, all tagged Greybox:vault.

Owner: WP-VAULT (Grok, Design/POIs/handoffs/sombre_WP-VAULT.md) if Anthony's go is relayed; otherwise Claude in VS-14.
The builder replaces this file. To take over the stub, declare GREYBOX_RETIRE_GROUPS = ("vault",) and build, in this
file only: the six spaces, VaultHatch_In / VaultLower_In / VaultConduit_In (to the ledger's anchors), the sombre.vault
location volume, and every gameplay actor inside the vault.

Called by build_pointe_sombre.py as build(tk). Owns only actors tagged Cell:vault.
"""

CELL = "vault"


def build(tk):
    tk.log("vault: not built yet (the greybox's stub vault stands in)")
