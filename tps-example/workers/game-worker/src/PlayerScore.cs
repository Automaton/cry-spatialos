using System;
using Automaton.Player;
using Improbable;
using Improbable.Worker;

class PlayerScore
{
    private readonly Dispatcher _dispatcher;
    private readonly Connection _connection;
    private readonly EntityId _entityId;
    private readonly GameWorker _game;

    private uint _kills;
    private uint _deaths;

    public PlayerScore(GameWorker game, EntityId id, Dispatcher dispatcher, Connection connection)
    {
        _game = game;
        _dispatcher = dispatcher;
        _connection = connection;
        _entityId = id;
        Initialise();
    }

    public void Initialise()
    {
        _dispatcher.OnAddComponent<Score>(op =>
        {
            if (op.EntityId != _entityId) return;
            Console.WriteLine("Added PlayerScore to {0}", _entityId);
            OnUpdate(op.Data.ToUpdate().Get());

        });

        _dispatcher.OnComponentUpdate<Score>(op =>
        {
            if (op.EntityId != _entityId) return;
            OnUpdate(op.Update.Get());
        });
    }

    void OnUpdate(Score.Update update)
    {
        if (update.deaths.HasValue)
        {
            _deaths = update.deaths.Value;
        }
        if (update.kills.HasValue)
        {
            _kills = update.kills.Value;
        }
    }

    public void AddKill(uint count = 1)
    {
        Score.Update update = new Score.Update();
        update.SetKills(_kills + count);
        _connection.SendComponentUpdate(_entityId, update);
    }

    public void AddDeath(uint count = 1)
    {
        Score.Update update = new Score.Update();
        update.SetDeaths(_deaths + count);
        _connection.SendComponentUpdate(_entityId, update);
    }
}