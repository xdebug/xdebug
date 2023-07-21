<?php

namespace Service;

class Example
{
    public function executeAction(): bool
    {
        $path = $this->getBasicPath()
            . '/'
            . $this->getFileName();

        return $this->runAction($path);
    }

    private function getBasicPath(): string
    {
        return '/var/www';
    }

    private function getFileName(): string
    {
        return 'file.csv';
    }

    private function runAction(string $path): bool
    {
        return true;
    }
}
