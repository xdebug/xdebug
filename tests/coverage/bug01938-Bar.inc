<?php

declare(strict_types=1);

namespace App;

class Bar
{
    use FooTrait;

    public function useTrait(): bool
    {
        return $this->returnsTrue();
    }
}
