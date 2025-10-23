<?php
class User
{
	public int $id;
	public string $name;
}

$reflection = new ReflectionClass(User::class);

$user = $reflection->newLazyProxy(function (User $user): User {
	$new = new User();
	$new->id = 43;
	$new->name = "Bluey";

	return $new;
});

var_dump($user->id);

var_dump($reflection->isUninitializedLazyObject($user));
