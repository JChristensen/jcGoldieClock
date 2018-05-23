//miscellaneous classes

/*----------------------------------------------------------------------*
 * heartbeat LED class                                                  *
 *----------------------------------------------------------------------*/
class heartbeat
{
public:
    heartbeat(uint8_t pin, uint32_t interval)
        : _pin(pin), _interval(interval), _state(true) {}
    void begin();
    void update();

private:
    uint8_t _pin;
    uint32_t _interval;
    uint32_t _lastHB;
    bool _state;
};

void heartbeat::begin()
{
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, _state);
    _lastHB = millis();
}

void heartbeat::update()
{
    if ( millis() - _lastHB >= _interval )
    {
        _lastHB += _interval;
        digitalWrite( _pin, _state = !_state);
    }
}

