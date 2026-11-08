# -*- coding: utf-8 -*-
# T16 part 3: clean repair — InitGame keeps only the delayed timer call; spawn body moves to SpawnEnemiesAndBoss().
import io
BASE = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

p = BASE + 'SalamanderGameMode.cpp'
s = io.open(p, encoding='utf-8').read()

start_anchor = '''	// Spawn simple enemies (server only). Count via -EnemyCount=<n> (default 2).
	// Delayed a few seconds so the navmesh is built before we pick random navigable points.'''
end_anchor = '''	// Periodic save every 60 s.
	GetWorldTimerManager().SetTimer(SaveTimerHandle, this, &ASalamanderGameMode::SaveAllProfiles, 60.f, true);'''
i0 = s.index(start_anchor)
i1 = s.index(end_anchor)
old_region = s[i0:i1]

# extract the spawn body from the old region (between the timer if-block and the stray closing brace)
body_start = old_region.index('\t\tint32 EnemyCount = 2;')
body_end = old_region.rindex('\n\t}\n')  # stray closing brace of the old if
spawn_body = old_region[body_start:body_end]

timer_block = '''	// Spawn simple enemies + boss (server only), delayed so the navmesh is built first.
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		FTimerHandle SpawnHandle;
		GetWorldTimerManager().SetTimer(SpawnHandle, this, &ASalamanderGameMode::SpawnEnemiesAndBoss, 3.f, false);
	}

'''
s = s[:i0] + timer_block + s[i1:]

# append the new function at the end of the file
new_func = '''
void ASalamanderGameMode::SpawnEnemiesAndBoss()
{
	if (GetWorld() == nullptr || GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}
''' + spawn_body + '''}
'''
s = s.rstrip() + '\n' + new_func
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('cpp repaired')

# header
p = BASE + 'SalamanderGameMode.h'
s = io.open(p, encoding='utf-8').read()
if 'SpawnEnemiesAndBoss' not in s:
    anchor = 'void ConnectToServer(const FString& Ip, const FString& Nick, const FString& Color);'
    assert anchor in s
    s = s.replace(anchor, anchor + '\n\n\t// Spawns enemies + boss on random navigable points (called 3 s after InitGame, server only).\n\tvoid SpawnEnemiesAndBoss();', 1)
    io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('h ok')
print('patch_t16c OK')
