using System;
using Automaton.Player;
using Improbable;
using Improbable.Worker;

public class Player
{
    private readonly Dispatcher _dispatcher;
    private readonly Connection _connection;
    private readonly EntityId _entityId;
    private readonly GameWorker _game;

    private readonly PlayerScore _score;

    private const int HeartbeatInterval = 3000;
    private const int HeartbeatTimeout = HeartbeatInterval * 5;
    private int _lastHeartbeat;
    private bool _ready;

    public EntityId entityId => _entityId;


    public Player(GameWorker game, EntityId id, Dispatcher dispatcher, Connection connection)
    {
        _score = new PlayerScore(game, id, dispatcher, connection);
        _game = game;
        _dispatcher = dispatcher;
        _connection = connection;
        _entityId = id;
    }

    public void Initialise()
    {
        _lastHeartbeat = Environment.TickCount;
        _ready = true;
        _dispatcher.OnComponentUpdate<Automaton.Player.Player>(OnPlayerUpdate);
    }

    private void OnPlayerUpdate(ComponentUpdateOp<Automaton.Player.Player> op)
    {
        if (op.EntityId != _entityId) return;
        IComponentUpdate<Automaton.Player.Player> update = op.Update;
        Automaton.Player.Player.Update u = update.Get();
        if (u.heartbeat.Count > 0)
        {
            _lastHeartbeat = Environment.TickCount;
        }
        if (u.killedPlayer.Count > 0)
        {
            foreach (var kp in u.killedPlayer)
            {
                var killedEntity = kp.killed;
                Player player = _game.GetPlayer(killedEntity);
                if (player == null)
                {
                    Console.WriteLine("Failed to find player ({0}) to kill", killedEntity);
                }
                else
                {
                    _score.AddKill();
                    player._score.AddDeath();
                }
            }
        }
    }

    public bool Update()
    {
        if (!_ready) return true;
        if (Environment.TickCount - _lastHeartbeat > HeartbeatTimeout)
        {
            Console.WriteLine("Timing out player with entity ID {0}", _entityId);
            _connection.SendDeleteEntityRequest(_entityId, 3000);
            return false;
        }
        return true;
    }
}
