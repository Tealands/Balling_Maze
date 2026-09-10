using UnityEngine;

public class EnemyAI : MonoBehaviour
{
    public float speed = 2.0f;
    public float wallMultiplier = 3.0f;
    public Transform target;

    void Start()
    {
        if (target == null && GameObject.FindWithTag("Player") != null)
            target = GameObject.FindWithTag("Player").transform;
    }

    void Update()
    {
        if (target == null) return;
        Vector3 dir = target.position - transform.position;
        dir.y = 0;
        float dist = dir.magnitude;
        if (dist < 0.001f) return;
        dir.Normalize();

        // simple adjacency check for walls: raycasts in 4 directions
        bool adjacentWall = false;
        RaycastHit hit;
        Vector3[] checks = new Vector3[] { Vector3.forward, Vector3.back, Vector3.left, Vector3.right };
        foreach (var c in checks)
        {
            if (Physics.Raycast(transform.position, c, out hit, 1.1f))
            {
                if (hit.collider != null && hit.collider.gameObject.CompareTag("Wall")) { adjacentWall = true; break; }
            }
        }

        float spd = speed * (adjacentWall ? wallMultiplier : 1.0f);
        Vector3 newPos = transform.position + dir * spd * Time.deltaTime;
        // simple move
        transform.position = newPos;
    }
}
