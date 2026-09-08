# Fuzzy Search Algorithm & Scoring Model

This document outlines the scoring heuristics, prefix boosting, and boundary matching algorithms employed by Orca Light.

---

## 1. Algorithmic Overview

Orca Light utilizes a custom, cache-conscious fuzzy matcher designed specifically for command-palette and application-launcher query patterns. Rather than standard Levenshtein distance—which penalizes insertions uniformly and performs poorly on abbreviations—Orca Light combines contiguous subsequence tracking with context-aware boundary weighting.

---

## 2. Mathematical Scoring Rules

The total match score $S$ for a candidate target string $T$ given a user query $Q$ is computed as:

$$S(T, Q) = \sum_{i=1}^{|Q|} \text{CharScore}(q_i) + \text{StreakBonus} + \text{PrefixBonus} - \text{DepthPenalty}$$

### 2.1 Character Match Weights
When a character $q_i$ in query $Q$ matches character $t_j$ in target $T$:

| Match Condition | Point Weight | Rationale |
| :--- | :--- | :--- |
| **Exact Case Match** | +100 | User matched uppercase/lowercase intentionally |
| **Case-Insensitive Match** | +75 | Standard match credit |
| **Prefix Match ($j = 0$)** | +500 | Matching the first character is a primary signal of user intent |
| **Word Boundary Match** | +250 | Character follows a delimiter (` `, `.`, `-`, `_`, `\`, `/`) |
| **CamelCase Boundary** | +200 | Transition from lowercase to uppercase (e.g., `o` to `L` in `OrcaLight`) |

### 2.2 Streak Multiplier
Sequential matched characters indicate tight semantic alignment rather than scattered character coincidences. A contiguous run counter accumulates across consecutive matches:
- 1st sequential character: +50
- 2nd sequential character: +150
- 3rd sequential character: +300
- 4th+ sequential character: +500

### 2.3 Directory Depth Penalty
For file results, deeply nested files often generate false-positive matches for common letters. A penalty is deducted for each path delimiter in the target:
$$\text{DepthPenalty} = \text{DirectoryLevels} \times 10$$

---

## 3. Heuristic Filtering & Edge Cases

### 3.1 Acronym & Abbreviation Matching
Orca Light naturally favors acronyms. For example, querying `vs` against `Visual Studio Code` achieves:
- `V`: Matches index 0 (Prefix Bonus +500, Exact Case +100) = 600 points.
- `s`: Matches index 7 (Word Boundary Bonus +250, Case-Insensitive +75) = 325 points.
- Total Score: 925 points.
This guarantees acronyms rank above generic files containing incidental letters `v` and `s`.

### 3.2 Application vs File Prioritization
To prevent user documents from obscuring primary executables:
$$\text{FinalScore} = \text{RawScore} \times \text{TypeMultiplier}$$
- Installed Application: `TypeMultiplier = 2.0`
- System Command: `TypeMultiplier = 1.8`
- User Document: `TypeMultiplier = 1.0`
