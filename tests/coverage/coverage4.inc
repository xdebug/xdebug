<?php
interface ezcFeedParser                                                         
{                                                                               
    /**                                                                         
     * Returns true if the parser can parse the provided XML document object,   
     * false otherwise.                                                         
     *                                                                          
     * @param DOMDocument $xml The XML document object to check for parseability
     * @return bool                                                             
     */                                                                         
    public static function canParse( DOMDocument $xml );                        
                                                                                
    /**                                                                         
     * Parses the provided XML document object and returns an ezcFeed object    
     * from it.                                                                 
     *                                                                          
     * @throws ezcFeedParseErrorException                                       
     *         If an error was encountered during parsing.                      
     *                                                                          
     * @param DOMDocument $xml The XML document object to parse                 
     * @return ezcFeed                                                          
     */                                                                         
    public function parse( DOMDocument $xml );                                  
}                                                                               
?>     
