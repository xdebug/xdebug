<?php
class User
{
	public int $id;
	public string $name;
}

$reflection = new ReflectionClass(User::class);

$user = $reflection->newLazyGhost(function (User $user): void {
	throw new \RuntimeException('Initialization should not be called');
});

var_dump($reflection->isUninitializedLazyObject($user));
