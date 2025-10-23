<?php
class User
{
	public int $id;
	public string $name;
}

$reflection = new ReflectionClass(User::class);

$user = $reflection->newLazyGhost(function (User $user): void {
	$user->id = 42;
	$user->name = "Elephpant";
});

var_dump($user->id);

var_dump($reflection->isUninitializedLazyObject($user));
