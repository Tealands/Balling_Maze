using System;
using System.IO;
using System.Collections.Generic;
using UnityEngine;

// Simple map importer that reads StreamingAssets/maps/map*.txt and spawns cubes for walls, keys, start and goal.
// Usage: attach to an empty GameObject in the scene and call Import("map1.txt") from inspector or Awake.

public class MapImporter : MonoBehaviour
{
    public string mapFile = "map1.txt"; // relative to StreamingAssets/maps/

    public GameObject wallPrefab; // optional, if null uses cube
    public GameObject keyPrefab;
    public GameObject goalPrefab;
    public GameObject playerPrefab;
    public GameObject enemyPrefab;

    private List<GameObject> spawned = new List<GameObject>();

    public void Import(string filename = null)
    {
        string fname = filename ?? mapFile;
        string dir = Path.Combine(Application.streamingAssetsPath, "maps");
        string path = Path.Combine(dir, fname);
        if (!File.Exists(path))
        {
            Debug.LogError("Map file not found: " + path);
            return;
        }

        foreach (var go in spawned) DestroyImmediate(go);
        spawned.Clear();

        var lines = File.ReadAllLines(path);
        int h = lines.Length;
        for (int r = 0; r < h; ++r)
        {
            var row = lines[r];
            for (int c = 0; c < row.Length; ++c)
            {
                char ch = row[c];
                Vector3 pos = new Vector3(c + 0.5f, 0.5f, r + 0.5f);
                if (ch == '#')
                {
                    GameObject wall = wallPrefab != null ? Instantiate(wallPrefab, pos, Quaternion.identity) : GameObject.CreatePrimitive(PrimitiveType.Cube);
                    if (wallPrefab == null) wall.transform.position = pos;
                    // ensure wall tag and collider
                    wall.tag = "Wall";
                    if (wall.GetComponent<Collider>() == null) wall.AddComponent<BoxCollider>();
                    spawned.Add(wall);
                }
                else if (ch == 'S')
                {
                    if (playerPrefab != null)
                    {
                        GameObject p = Instantiate(playerPrefab, pos, Quaternion.identity);
                        p.tag = "Player";
                        spawned.Add(p);
                    }
                    else
                    {
                        // create placeholder sphere
                        GameObject ps = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                        ps.transform.position = pos;
                        ps.name = "PlayerPlaceholder";
                        // add CharacterController for movement
                        var cc = ps.AddComponent<CharacterController>();
                        ps.tag = "Player";
                        spawned.Add(ps);
                    }
                }
                else if (ch == 'G')
                {
                    if (goalPrefab != null)
                    {
                        GameObject g = Instantiate(goalPrefab, pos, Quaternion.identity);
                        g.tag = "Goal";
                        spawned.Add(g);
                    }
                    else
                    {
                        GameObject gp = GameObject.CreatePrimitive(PrimitiveType.Cube);
                        gp.transform.position = pos;
                        gp.name = "GoalPlaceholder";
                        gp.tag = "Goal";
                        spawned.Add(gp);
                    }
                }
                else if (ch == 'a' || ch == 'b' || ch == 'c')
                {
                    if (keyPrefab != null)
                    {
                        GameObject k = Instantiate(keyPrefab, pos, Quaternion.identity);
                        k.tag = "Key";
                        spawned.Add(k);
                    }
                    else
                    {
                        GameObject kp = GameObject.CreatePrimitive(PrimitiveType.Sphere);
                        kp.transform.position = pos;
                        kp.name = "Key_" + ch;
                        kp.tag = "Key";
                        spawned.Add(kp);
                    }
                }
                else if (ch == 'A' || ch == 'B' || ch == 'C')
                {
                    if (enemyPrefab != null)
                    {
                        GameObject e = Instantiate(enemyPrefab, pos, Quaternion.identity);
                        e.tag = "Enemy";
                        spawned.Add(e);
                    }
                    else
                    {
                        GameObject ep = GameObject.CreatePrimitive(PrimitiveType.Cube);
                        ep.transform.position = pos;
                        ep.name = "EnemySpawn_" + ch;
                        ep.tag = "Enemy";
                        spawned.Add(ep);
                    }
                }
            }
        }

        Debug.Log("Map imported: " + path);
    }
}
