using System;
using System.Collections.Generic;
using Automaton.Player;
using Automaton;
using Improbable;
using Improbable.Collections;
using Improbable.Worker;

class Spawner
{

    private readonly EntityId _entityId;
    private readonly Dispatcher _dispatcher;
    private readonly Connection _connection;

    private readonly Dictionary<RequestId<ReserveEntityIdsRequest>, CommandRequestOp<Automaton.Spawner.Commands.SpawnPlayer>> _spawnPlayerRequests;
    private readonly Dictionary<RequestId<CreateEntityRequest>, CommandRequestOp<Automaton.Spawner.Commands.SpawnPlayer>> _pendingCreateEntities;

    private bool _isReady;

    public Spawner(EntityId entityId, Dispatcher dispatcher, Connection connection)
    {
        _entityId = entityId;
        _dispatcher = dispatcher;
        _connection = connection;

        _spawnPlayerRequests = new Dictionary<RequestId<ReserveEntityIdsRequest>, CommandRequestOp<Automaton.Spawner.Commands.SpawnPlayer>>();
        _pendingCreateEntities = new Dictionary<RequestId<CreateEntityRequest>, CommandRequestOp<Automaton.Spawner.Commands.SpawnPlayer>>();
        RegisterCallbacks();
    }

    private void RegisterCallbacks()
    {
        _dispatcher.OnAddComponent<Automaton.Spawner>(op =>
        {
            _isReady = true;
        });
        _dispatcher.OnCommandRequest<Automaton.Spawner.Commands.SpawnPlayer>(op =>
        {
            Console.WriteLine("Received spawn player request from {0}", op.CallerWorkerId);
            // TODO: Would be nice to reserve a certain number of entity IDs for players and reuse them instead
            _spawnPlayerRequests.Add(_connection.SendReserveEntityIdsRequest(1, 3000), op);
        });
        _dispatcher.OnReserveEntityIdsResponse(op =>
        {
            CommandRequestOp<Automaton.Spawner.Commands.SpawnPlayer> request;
            if (!_spawnPlayerRequests.TryGetValue(op.RequestId, out request)) return;
            _spawnPlayerRequests.Remove(op.RequestId);
            if (op.StatusCode != StatusCode.Success)
            {
                Console.WriteLine("Failed to reserve ID for request {0}, status code: {1}, reason: {2}", request.RequestId, op.StatusCode, op.Message);
            }
            else
            {
                
                Console.WriteLine("Requesting entity creation: {0}", op.FirstEntityId.Value);
                Entity entity = CreatePlayerTemplate(request, request.Request.Get().Value.position);
                RequestId<CreateEntityRequest> id = _connection.SendCreateEntityRequest(entity, op.FirstEntityId.Value, 1000);
                _pendingCreateEntities.Add(id, request);

            }
        });
        _dispatcher.OnCreateEntityResponse(op =>
        {
            CommandRequestOp<Automaton.Spawner.Commands.SpawnPlayer> request;
            if (!_pendingCreateEntities.TryGetValue(op.RequestId, out request)) return;
            _pendingCreateEntities.Remove(op.RequestId);
            if (op.StatusCode == StatusCode.Success)
            {
                Automaton.Spawner.Commands.SpawnPlayer.Response response =
                    new Automaton.Spawner.Commands.SpawnPlayer.Response(true, "", op.EntityId.Value);
                Console.WriteLine("Successfully spawned player entity: {0}", op.EntityId.Value);
                _connection.SendCommandResponse(request.RequestId, response);
            }
            else
            {
                Console.WriteLine("Failed to spawn entity: {0}", op.Message);
                Automaton.Spawner.Commands.SpawnPlayer.Response response = new Automaton.Spawner.Commands.SpawnPlayer.Response(false, op.Message, new EntityId(0));
                _connection.SendCommandResponse(request.RequestId, response);
            }
        });
    }

    private static Entity CreatePlayerTemplate(CommandRequestOp<Automaton.Spawner.Commands.SpawnPlayer> op, Coordinates position)
    {
        var callerWorkerRequirementSet = new WorkerRequirementSet(new Improbable.Collections.List<WorkerAttributeSet>{ new WorkerAttributeSet(op.CallerAttributeSet) });
        var gameWorkerAttr = new WorkerAttributeSet(new Improbable.Collections.List<string> { "GameWorker" });
        var clientWorkerAttr = new WorkerAttributeSet(new Improbable.Collections.List<string>() { "TPSClient" });

        var gameWorkerRequirementSet = new WorkerRequirementSet(new Improbable.Collections.List<WorkerAttributeSet> { gameWorkerAttr });

        var allRequirementSet = new WorkerRequirementSet(new Improbable.Collections.List<WorkerAttributeSet>
        {
            clientWorkerAttr,
            gameWorkerAttr
        });
        var writeAcl = new Map<uint, WorkerRequirementSet>
        {
            { Position.ComponentId, callerWorkerRequirementSet  },
            { EntityAcl.ComponentId, gameWorkerRequirementSet   },
            { Movement.ComponentId, callerWorkerRequirementSet  },
            { Automaton.Player.Player.ComponentId, callerWorkerRequirementSet },
            { Score.ComponentId, gameWorkerRequirementSet }
        };
        Entity entity = new Entity();
        entity.Add(new EntityAcl.Data(/* read */ allRequirementSet, /* write */ writeAcl));
        entity.Add(new Metadata.Data("SpatialOsPlayer"));
        entity.Add(new Position.Data(position));
        entity.Add(new Movement.Data(false, new Quaternion(1, 0, 0, 0), false));
        entity.Add(new Automaton.Player.Player.Data());
        entity.Add(new Score.Data(0, 0)); 
        return entity;
    }

    public void Update()
    {
        
    }
}