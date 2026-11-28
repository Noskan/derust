//! Gera o código do contrato gRPC (proto/w2db/v1/w2db.proto) com o protox, sem depender do protoc.

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let proto = "proto/w2db/v1/w2db.proto";
    println!("cargo:rerun-if-changed={proto}");
    let fds = protox::compile([proto], ["proto"])?;
    tonic_prost_build::configure()
        // As mensagens também vão para o jsonb do banco (dados do personagem e versões da conta).
        .type_attribute(".w2db.v1", "#[derive(serde::Serialize, serde::Deserialize)]")
        .message_attribute(".w2db.v1", "#[serde(default)]")
        .compile_fds(fds)?;
    Ok(())
}
