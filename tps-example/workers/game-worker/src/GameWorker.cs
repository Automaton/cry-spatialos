using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Improbable;
using Improbable.Worker;
using Improbable.Worker.Query;

public class GameWorker
{

    const int FramesPerSecond = 60;
    const int BulletTimeout = 10000;

    private readonly Connection _connection;
    private readonly Dispatcher _dispatcher;
    private Spawner _spawner;

    private System.Collections.Generic.List<Player> _players;
    private SortedSet<EntityId> _spawnedEntities;
    private Dictionary<EntityId, long> _bullets;

    private EntityId _spawnerId;

    static int Main(string[] args)
    {
        if (args.Length < 1)
        {
            return 1;
        }
       Assembly.Load("GeneratedCode");
        var parameters = new ConnectionParameters
        {
            WorkerType = "GameWorker",
            Network =
            {
                ConnectionType = NetworkConnectionType.Tcp,
                UseExternalIp = false
            }
        };

        var hostname = "localhost";
        if (args.Length >= 2)
        {
            hostname = args[1];
        }

        var port = 7777;
        if (args.Length >= 3)
        {
            port = int.Parse(args[2]);
        }
        var connection = Connection.ConnectAsync(hostname, (ushort) port, args[0], parameters).Get();
        GameWorker worker = new GameWorker(connection, new View());
        worker.RunEventLoop();
        return 0;
    }

    private GameWorker(Connection connection, Dispatcher dispatcher)
    {
        _connection = connection;
        _dispatcher = dispatcher;
        _spawnedEntities = new SortedSet<EntityId>(Comparer<EntityId>.Create((e1, e2) => e1.Id.CompareTo(e2.Id)));
        _players = new System.Collections.Generic.List<Player>();
        _bullets = new Dictionary<EntityId, long>();
        Setup();
    }

    private void Setup()
    {
        _dispatcher.OnAddEntity(op =>
        {
            Console.WriteLine("AddEntity {0}", op.EntityId);
            _spawnedEntities.Add(op.EntityId);
        });
        _dispatcher.OnRemoveEntity(op =>
        {
            var id = op.EntityId;
            Console.WriteLine("RemoveEntity {0}", id);
            _spawnedEntities.Remove(id);
            _bullets.Remove(id);
        });
        _dispatcher.OnAddComponent<Metadata>(op =>
        {
            Console.WriteLine("Add Metadata to {0}, name: {1}", op.EntityId, op.Data.Get().Value.entityType);
            if (!_spawnedEntities.Contains(op.EntityId))
            {
                Console.WriteLine("Tried to add metadata for a non-spawned entity");
                return;
            }
            string type = op.Data.Get().Value.entityType;
            if (type == "Spawner")
            {
                _spawnerId = op.EntityId;
                _spawner = new Spawner(op.EntityId, _dispatcher, _connection);
            }
        });
        _dispatcher.OnAddComponent<Automaton.Player.Player>(op =>
        {
            Console.WriteLine("Adding Player Component to {0}", op.EntityId);
            if (!_spawnedEntities.Contains(op.EntityId))
            {
                Console.WriteLine("Tried to add player component for a non-spawned entity");
                return;
            }
            var player = new Player(this, op.EntityId, _dispatcher, _connection);
            _players.Add(player);
            player.Initialise();
        });
        _dispatcher.OnAddComponent<Automaton.Player.Bullet>(op =>
        {
            _bullets.Add(op.EntityId, Environment.TickCount);
        });
    }

    public Player GetPlayer(EntityId id)
    {
        return _players.Find(p => p.entityId == id);
    }

    void RunEventLoop()
    {
        var nextFrameTime = System.DateTime.Now;
        while (_connection.IsConnected)
        {
            long tick = Environment.TickCount;
            var opList = _connection.GetOpList(0 /* non-blocking */);
            // Invoke user-provided callbacks.
            _dispatcher.Process(opList);
            // Do other work here...
            _spawner?.Update();
            // Remove all players whose update fails
            _players.RemoveAll(p => !p.Update());
            // Process bullets (i.e. remove old ones)
            var timedOutBullets = _bullets.Where(kvp => (tick - kvp.Value) > 10000).Select(kvp => kvp.Key).ToList();
            if (timedOutBullets.Count > 0) Console.WriteLine("Timing out {0} bullets", timedOutBullets.Count);
            foreach (var id in timedOutBullets)
            {
                _bullets.Remove(id);
                _connection.SendDeleteEntityRequest(id, 3000);
            }
            nextFrameTime = nextFrameTime.AddMilliseconds(1000f / FramesPerSecond);
            var waitFor = nextFrameTime.Subtract(System.DateTime.Now);
            System.Threading.Thread.Sleep(waitFor.Milliseconds > 0 ? waitFor : System.TimeSpan.Zero);
        }
    }
}