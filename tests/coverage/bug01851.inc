<?php
class NPSRepository
{
    /** @var PDO */
    private $pdo;

    public function __construct(PDO $pdo)
    {
        $this->pdo = $pdo;
        $this->pdo->exec('CREATE TABLE np_score(user_id INTEGER NOT NULL, score INTEGER NOT NULL);');
    }

    public function importUserRecords(array $records): void
    {
        $statement = $this->pdo->prepare('INSERT INTO np_score(user_id, score) VALUES (:userId, :score)');
        foreach ($records as $userId => $score) {
            $statement->bindValue('userId', $userId, PDO::PARAM_INT);
            $statement->bindValue('score', $score, PDO::PARAM_INT);
            $statement->execute();
        }
    }
}

$pdo = new PDO('sqlite::memory:', null, null, [PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION]);
$repository = new NPSRepository($pdo);

$repository->importUserRecords([1,1]);
?>
