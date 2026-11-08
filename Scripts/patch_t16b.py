# -*- coding: utf-8 -*-
# T16 part 2: move enemy/boss spawn into SpawnEnemiesAndBoss() called after a 3s delay (navmesh ready).
import io
BASE = 'C:/Users/Blackkus/Documents/Unreal Projects/Salamander/Source/Salamander/'

p = BASE + 'SalamanderGameMode.cpp'
s = io.open(p, encoding='utf-8').read()

old_block = '''	// Spawn simple enemies (server only). Count via -EnemyCount=<n> (default 2).
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		int32 EnemyCount = 2;'''
new_block = '''	// Spawn simple enemies (server only), delayed so the navmesh is built first.
	if (GetWorld()->GetNetMode() != NM_Client)
	{
		FTimerHandle SpawnHandle;
		GetWorldTimerManager().SetTimer(SpawnHandle, this, &ASalamanderGameMode::SpawnEnemiesAndBoss, 3.f, false);
	}

	// Periodic save every 60 s.'''
assert old_block in s
# cut everything from old_block up to the closing of the if block (the line "\t}\n\n\t// Periodic save")
end_marker = '''		if (BossCount > 0)
		{
			UE_LOG(LogTemp, Log, TEXT("[GameMode] spawned %d boss"), BossCount);
		}
	}

	// Periodic save every 60 s.'''
assert end_marker in s
spawn_body = s[s.index(old_block):s.index(end_marker) + len(end_marker)]
# build the new function body from the old block (strip the outer if wrapper)
inner = spawn_body[len(old_block):]
inner = inner[:inner.rindex('\t}\n\n\t// Periodic save every 60 s.')]
new_func = '''
void ASalamanderGameMode::SpawnEnemiesAndBoss()
{
	if (GetWorld() == nullptr || GetWorld()->GetNetMode() == NM_Client)
	{
		return;
	}
''' + inner + '''}
'''
s = s.replace(spawn_body, new_block, 1)
# append the new function right after InitGame's closing — simpler: append at end of file
s += new_func
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('cpp ok')

# header declaration
p = BASE + 'SalamanderGameMode.h'
s = io.open(p, encoding='utf-8').read()
anchor = 'void ConnectToServer(const FString& Ip, const FString& Nick, const FString& Color);'
assert anchor in s
s = s.replace(anchor, anchor + '\n\n\t// Spawns enemies + boss on random navigable points (called 3 s after InitGame, server only).\n\tvoid SpawnEnemiesAndBoss();', 1)
io.open(p, 'w', encoding='utf-8', newline='').write(s)
print('h ok')
print('patch_t16b OK')
