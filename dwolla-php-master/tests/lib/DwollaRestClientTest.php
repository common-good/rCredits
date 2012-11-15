<?php

class DwollaRestClientTest extends \PHPUnit_Framework_TestCase
{

    /**
     * @var DwollaRestClient
     */
    private $client;

    /**
     * @var array 
     */
    private $config;

    public function setUp()
    {
        $this->config = include __DIR__ . '/../config.php';
        $this->client = new DwollaRestClient(
                $this->config['apiKey'],
                $this->config['apiSecret'],
                $this->config['redirectUri'],
                $permissions = array("send", "transactions", "balance", "request", "contacts", "accountinfofull", "funding"),
                'test'
        );
    }

    public function testGetAuthUrl()
    {
        $authUrl = $this->client->getAuthUrl();

        $components = parse_url($authUrl);
        $this->assertEquals('https', $components['scheme']);
        $this->assertEquals('www.dwolla.com', $components['host']);
        $this->assertEquals('/oauth/v2/authenticate', $components['path']);

        parse_str($components['query'], $query);
        $this->assertEquals($this->config['apiKey'], $query['client_id']);
        $this->assertEquals('code', $query['response_type']);
        $this->assertEquals('send|transactions|balance|request|contacts|accountinfofull|funding', $query['scope']);
        $this->assertEquals($this->config['redirectUri'], $query['redirect_uri']);
    }

    public function testRequestToken()
    {
        $this->markTestIncomplete("Can't test without a valid oauth code.");
    }

    public function testRequestTokenNoCodeProvided()
    {
        $this->markTestIncomplete('Warning thrown if no token provided. Method must be refactored before expected behavior is obtained.');
        // TODO throw exception if oauth code isn't provided
        $this->assertFalse($this->client->requestToken());
        $this->assertEquals('Please pass an oauth code.', $this->client->getError());
    }

    public function testMe()
    {
        // TODO Unclear that token must be set, no error/exception thrown 
        // if token isn't provided, and I don't like the existing error message
        // mechanism at all.  Should be refactored
        $this->client->setToken($this->config['token']);
        $this->assertEquals($this->config['me'], $this->client->me());
    }

    public function testGetUser()
    {
        $user = $this->client->getUser($this->config['me']['Id']);
        $expected = array(
            'Id' => $this->config['me']['Id'],
            'Name' => $this->config['me']['Name'],
            'Latitude' => $this->config['me']['Latitude'],
            'Longitude' => $this->config['me']['Longitude']
        );
        $this->assertEquals($expected, $user);
    }

    public function testRegister()
    {
        $this->markTestIncomplete('Test not implemented');
    }

    public function testContacts()
    {
        $this->markTestIncomplete('As I have no contacts, this is impossible to test');
        // TODO should throw exception if no token provided
        $this->client->setToken($this->config['token']);
        $contacts = $this->client->contacts('Ben');
    }

    public function testNearbyContacts()
    {
        $this->markTestIncomplete('As I have no contacts, this is impossible to test');
    }

    public function testFundingSources()
    {
        $this->client->setToken($this->config['token']);
        $sources = $this->client->fundingSources();

        $this->assertInternalType('array', $sources);
        $this->assertTrue(count($sources) > 0, "Authenticated user doesn't have any funding sources");

        // Ensure the first source is an array and has the correct keys
        $this->assertInternalType('array', $sources[0]);
        $this->assertEquals(4, count($sources[0]));
        $this->assertArrayHasKey('Id', $sources[0]);
        $this->assertArrayHasKey('Name', $sources[0]);
        $this->assertArrayHasKey('Type', $sources[0]);
        $this->assertArrayHasKey('Verified', $sources[0]);
    }

    public function testFundingSource()
    {
        $this->markTestIncomplete("DwollaRestClient::fundingSource() improperly returns the same as DwollaRestClient::fundingSources()");
        $this->client->setToken($this->config['token']);
        // This funding id came from the examples. Doesn't matter b/c the 
        // fundingSource() method returns exactly the same as fundingSources()
        // Doubleplusungood
        $sourceId = 'pJRq4tK38fiAeQ8xo2iH9Q==';
        $source = $this->client->fundingSource($sourceId);
    }

    public function testBalance()
    {
        $this->client->setToken($this->config['token']);
        $balance = $this->client->balance();
        $this->assertInternalType('float', $balance);
        $this->markTestIncomplete('Provide more assertions once a test user is provided');
    }
    
    public function testParseDwollaId()
    {
        $id = '1234567890';
        $this->assertEquals('123-456-7890', $this->client->parseDwollaID($id));
        
        $id = 'id=123,456.7890';
        $this->assertEquals('123-456-7890', $this->client->parseDwollaID($id));
    }
    
    public function testSetModeThrowsExceptionIfNotLiveOrTest()
    {
        $this->setExpectedException('InvalidArgumentException', 'Appropriate mode values are live or test');
        $this->client->setMode('whatever');
    }
    
    public function testSetModeOKWithAppropriateValue()
    {
        $this->client->setMode('test');
        $this->assertEquals('test', $this->client->getMode());
    }

}
