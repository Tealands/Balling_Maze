using UnityEngine;

[RequireComponent(typeof(CharacterController))]
public class PlayerController : MonoBehaviour
{
    public float speed = 5.0f;
    public float jumpSpeed = 6.0f;
    public float gravity = 9.8f;

    private CharacterController cc;
    private Vector3 velocity = Vector3.zero;

    void Awake()
    {
        cc = GetComponent<CharacterController>();
    }

    void Update()
    {
        // movement relative to camera
        Vector3 forward = Camera.main.transform.forward;
        forward.y = 0; forward.Normalize();
        Vector3 right = Camera.main.transform.right;
        right.y = 0; right.Normalize();

        float v = 0f, h = 0f;
        if (Input.GetKey(KeyCode.W)) v += 1f;
        if (Input.GetKey(KeyCode.S)) v -= 1f;
        if (Input.GetKey(KeyCode.A)) h -= 1f;
        if (Input.GetKey(KeyCode.D)) h += 1f;

        Vector3 move = (forward * v + right * h);
        if (move.magnitude > 1f) move.Normalize();

        Vector3 desired = move * speed;
        velocity.x = desired.x;
        velocity.z = desired.z;

        if (cc.isGrounded)
        {
            if (Input.GetKeyDown(KeyCode.Space)) velocity.y = jumpSpeed;
            else velocity.y = -0.1f;
        }
        velocity.y -= gravity * Time.deltaTime;

        cc.Move(velocity * Time.deltaTime);
    }
}
