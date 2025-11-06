<?php
class User
{
    public int $id;
    public User $subUser;
}

$user = new User();
$reflection = new ReflectionClass(User::class);
$user->subUser = $reflection->newLazyProxy(function (User $user): User {
    return new User();
});

var_dump($user);
