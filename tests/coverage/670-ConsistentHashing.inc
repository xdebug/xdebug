<?php

/**
 * @author Kijin Sung <kijinbear@gmail.com>
 * @package Rediska
 * @subpackage Key distributor
 * @link http://github.com/kijin/distrib
 * @version 0.1.1
 * @link http://rediska.geometria-lab.net
 * @license http://www.opensource.org/licenses/bsd-license.php
 */
class Rediska_KeyDistributor_ConsistentHashing
{
    protected $_backends = array();
    protected $_backendsCount = 0;

    protected $_hashring = array();
    protected $_hashringCount = 0;

    protected $_replicas = 256;
    protected $_slicesCount = 0;
    protected $_slicesHalf = 0;
    protected $_slicesDiv = 0;

    protected $_cache = array();
    protected $_cacheCount = 0;
    protected $_cacheMax = 256;

    protected $_hashringIsInitialized = false;

    /**
     * (non-PHPdoc)
     * @see Rediska_KeyDistributor_Interface#addConnection
     */
    public function addConnection($connectionString, $weight = Rediska_Connection::DEFAULT_WEIGHT)
    {
        if (isset($this->_backends[$connectionString])) {
            throw new Rediska_KeyDistributor_Exception("Connection '$connectionString' already exists.");
        }

        $this->_backends[$connectionString] = $weight;

        $this->_backendsCount++;

        $this->_hashringIsInitialized = false;

        return $this;
    }

    /**
     * (non-PHPdoc)
     * @see Rediska_KeyDistributor_Interface#removeConnection
     */
    public function removeConnection($connectionString)
    {
        if (!isset($this->_backends[$connectionString])) {
            throw new Rediska_KeyDistributor_Exception("Connection '$connectionString' not exist.");
        }

        unset($this->_backends[$connectionString]);

        $this->_backendsCount--;
        
        $this->_hashringIsInitialized = false;

        return $this;
    }
    
    /**
     * (non-PHPdoc)
     * @see Rediska_KeyDistributor_Interface#getConnectionByKeyName
     */
    public function getConnectionByKeyName($name)
    {
        $connections = $this->getConnectionsByKeyName($name, 1);
        if (empty($connections)) {
            throw new Rediska_KeyDistributor_Exception('No connections exist');
        }
        return $connections[0];
    }

    /**
     * Get a list of connections by key name
     *
     * @param string $name Key name
     * @param int $requestedCount The length of the list to return
     * @return array List of connections
     */
    public function getConnectionsByKeyName($name, $count)
    {
        // If we have only one backend, return it.
        if ($this->_backendsCount == 1) {
            return array_keys($this->_backends);
        }

        if (!$this->_hashringIsInitialized) {
            $this->_initializeHashring();
            $this->_hashringIsInitialized = true;
        }

        // If the key has already been mapped, return the cached entry.
        if ($this->_cacheMax > 0 && isset($this->_cache[$name])) {
            return $this->_cache[$name];
        }

        // If $count is greater than or equal to the number of available backends, return all.
        if ($count >= $this->_backendsCount) return array_keys($this->_backends);

        // Initialize the return array.
        $return = array();

        $crc32 = crc32($name);

        // Select the slice to begin with.
        $slice = floor($crc32 / $this->_slicesDiv) + $this->_slicesHalf;

        // This counter prevents going through more than 1 loop.
        $looped = false;

        while (true) {
            // Go through the hashring, one slice at a time.
            foreach ($this->_hashring[$slice] as $position => $backend) {
                // If we have a usable backend, add to the return array.
                if ($position >= $crc32) {
                    // If $count = 1, no more checks are necessary.
                    if ($count === 1) {
                        $return = array($backend);
                        break 2;
                    } elseif (!in_array($backend, $return)) {
                        $return[] = $backend;
                        if (count($return) >= $count) break 2;
                    }

                    $return = array($backend);
                    break 1;
                }
            }

            // Continue to the next slice.
            $slice++;

            // If at the end of the hashring.
            if ($slice >= $this->_slicesCount) {
                // If already looped once, something is wrong.
                if ($looped) {
                    break 2;
                }

                // Otherwise, loop back to the beginning.       
                $crc32 = -2147483648;
                $slice = 0;
                $looped = true;
            }
        }

        // Cache the result for quick retrieval in the future.
        if ($this->_cacheMax > 0) {
            // Add to internal cache.
            $this->_cache[$name] = $return;
            $this->_cacheCount++;

            // If the cache is getting too big, clear it.
            if ($this->_cacheCount > $this->_cacheMax) {
                $this->_cleanCache();
            }
        }

        // Return the result.

        return $return;
    }

    protected function _initializeHashring()
    {
        if ($this->_backendsCount < 2) {
            $this->_hashring = array();
            $this->_hashringCount = 0;

            $this->_slicesCount = 0;
            $this->_slicesHalf = 0;
            $this->_slicesDiv = 0;
        } else {
            $this->_slicesCount = ($this->_replicas * $this->_backendsCount) / 8;
            $this->_slicesHalf = $this->_slicesCount / 2;
            $this->_slicesDiv = (2147483648 / $this->_slicesHalf);

            // Initialize the hashring.
            $this->_hashring = array_fill(0, $this->_slicesCount, array());

            // Calculate the average weight.
            $avg = round(array_sum($this->_backends) / $this->_backendsCount, 2);

            // Interate over the backends.
            foreach ($this->_backends as $backend => $weight) {
                // Adjust the weight.
                $weight = round(($weight / $avg) * $this->_replicas);
    
                // Create as many replicas as $weight.
                for ($i = 0; $i < $weight; $i++) {
                    $position = crc32($backend . ':' . $i);
                    $slice = floor($position / $this->_slicesDiv) + $this->_slicesHalf;
                    $this->_hashring[$slice][$position] = $backend;
                }
            }

            // Sort each slice of the hashring.
            for ($i = 0; $i < $this->_slicesCount; $i++) {
                ksort($this->_hashring[$i], SORT_NUMERIC);
            }
        }

        $this->_cleanCache();
    }

    protected function _cleanCache()
    {
        $this->_cache = array();
        $this->_cacheCount = 0;
    }
}
